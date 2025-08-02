#include "r3m/core/document_processor.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>

namespace r3m {
namespace core {

DocumentProcessor::DocumentProcessor() {
    // Initialize modular components
    pipeline_ = std::make_unique<processing::PipelineOrchestrator>();
    quality_assessor_ = std::make_unique<quality::QualityAssessor>();
    format_processor_ = std::make_unique<formats::FormatProcessor>();
    
    // Initialize default configuration (will be overridden by config)
    batch_size_ = parallel::OptimizedThreadPool::get_optimal_batch_size();
    max_workers_ = std::thread::hardware_concurrency();
    if (max_workers_ == 0) {
        max_workers_ = 4;  // Fallback - should be overridden by config
    }
    
    // Initialize statistics
    stats_.total_files_processed = 0;
    stats_.successful_processing = 0;
    stats_.failed_processing = 0;
    stats_.filtered_out = 0;
    stats_.avg_processing_time_ms = 0.0;
    stats_.total_text_extracted = 0;
    stats_.avg_content_quality_score = 0.0;
    stats_.pdf_files_processed = 0;
    stats_.text_files_processed = 0;
    stats_.html_files_processed = 0;
}

bool DocumentProcessor::initialize(const std::unordered_map<std::string, std::string>& config) {
    config_ = config;
    
    // Initialize all modular components
    if (!pipeline_->initialize(config)) {
        return false;
    }
    
    if (!quality_assessor_->initialize(config)) {
        return false;
    }
    
    if (!format_processor_->initialize(config)) {
        return false;
    }
    
    // Load batch processing settings with optimal defaults
    auto it = config_.find("document_processing.batch_size");
    if (it != config_.end()) {
        batch_size_ = std::stoul(it->second);
    } else {
        // Use optimal batch size based on CPU cores
        batch_size_ = parallel::OptimizedThreadPool::get_optimal_batch_size();
    }
    
    it = config_.find("document_processing.max_workers");
    if (it != config_.end()) {
        max_workers_ = std::stoul(it->second);
    } else {
        // Use optimal worker count
        max_workers_ = std::thread::hardware_concurrency();
        if (max_workers_ == 0) {
            max_workers_ = 4;
        }
    }
    
    it = config_.find("document_processing.enable_chunking");
    if (it != config_.end()) {
        enable_chunking_ = (it->second == "true" || it->second == "1");
    }
    
    // Initialize optimized thread pool
    thread_pool_ = std::make_unique<parallel::OptimizedThreadPool>(max_workers_);
    
    // Initialize chunking components if enabled
    if (enable_chunking_) {
        initialize_chunking_components();
    }
    
    initialized_ = true;
    return true;
}

void DocumentProcessor::initialize_chunking_components() {
    // Create tokenizer
    tokenizer_ = std::make_shared<chunking::BasicTokenizer>(8192);
    
    // Create chunker with configuration
    auto config = create_chunker_config();
    chunker_ = std::make_unique<chunking::AdvancedChunker>(tokenizer_, config);
}

chunking::AdvancedChunker::Config DocumentProcessor::create_chunker_config() {
    chunking::AdvancedChunker::Config config;
    
    // Load chunking configuration from config map
    auto it = config_.find("chunking.enable_multipass");
    if (it != config_.end()) {
        config.enable_multipass = (it->second == "true" || it->second == "1");
    }
    
    it = config_.find("chunking.enable_large_chunks");
    if (it != config_.end()) {
        config.enable_large_chunks = (it->second == "true" || it->second == "1");
    }
    
    it = config_.find("chunking.enable_contextual_rag");
    if (it != config_.end()) {
        config.enable_contextual_rag = (it->second == "true" || it->second == "1");
    }
    
    it = config_.find("chunking.include_metadata");
    if (it != config_.end()) {
        config.include_metadata = (it->second == "true" || it->second == "1");
    }
    
    it = config_.find("chunking.chunk_token_limit");
    if (it != config_.end()) {
        config.chunk_token_limit = std::stoi(it->second);
    }
    
    it = config_.find("chunking.chunk_overlap");
    if (it != config_.end()) {
        config.chunk_overlap = std::stoi(it->second);
    }
    
    it = config_.find("chunking.mini_chunk_size");
    if (it != config_.end()) {
        config.mini_chunk_size = std::stoi(it->second);
    }
    
    it = config_.find("chunking.blurb_size");
    if (it != config_.end()) {
        config.blurb_size = std::stoi(it->second);
    }
    
    it = config_.find("chunking.large_chunk_ratio");
    if (it != config_.end()) {
        config.large_chunk_ratio = std::stoi(it->second);
    }
    
    it = config_.find("chunking.max_metadata_percentage");
    if (it != config_.end()) {
        config.max_metadata_percentage = std::stod(it->second);
    }
    
    it = config_.find("chunking.contextual_rag_reserved_tokens");
    if (it != config_.end()) {
        config.contextual_rag_reserved_tokens = std::stoi(it->second);
    }
    
    return config;
}

chunking::AdvancedChunker::DocumentInfo DocumentProcessor::create_document_info(
    const std::string& file_path, 
    const std::string& text_content,
    const std::unordered_map<std::string, std::string>& metadata) {
    
    chunking::AdvancedChunker::DocumentInfo doc_info;
    
    // Set basic document information
    doc_info.document_id = utils::TextUtils::get_file_name(file_path);
    doc_info.title = utils::TextUtils::get_file_name(file_path);
    doc_info.semantic_identifier = utils::TextUtils::get_file_name(file_path);
    doc_info.source_type = "file";
    doc_info.full_content = text_content;
    doc_info.metadata = metadata;
    
    // Calculate total tokens
    doc_info.total_tokens = static_cast<int>(tokenizer_->count_tokens(text_content));
    
    // Create sections from text content
    // For now, treat the entire content as one section
    // In a more sophisticated implementation, this would parse the document structure
    chunking::section_processing::DocumentSection section;
    section.content = text_content;
    section.link = file_path;
    section.token_count = doc_info.total_tokens;
    
    doc_info.sections.push_back(section);
    
    return doc_info;
}

chunking::ChunkingResult DocumentProcessor::process_document_with_chunking(const std::string& file_path) {
    if (!enable_chunking_ || !chunker_) {
        chunking::ChunkingResult result;
        result.failed_chunks = 1;
        result.successful_chunks = 0;
        return result;
    }
    
    // First, process the document normally to get text content
    auto doc_result = process_single_document(file_path);
    
    if (!doc_result.processing_success) {
        chunking::ChunkingResult result;
        result.failed_chunks = 1;
        result.successful_chunks = 0;
        return result;
    }
    
    // Create document info for chunking
    auto doc_info = create_document_info(file_path, doc_result.text_content, doc_result.metadata);
    
    // Process with chunker
    return chunker_->process_document(doc_info);
}

std::vector<chunking::ChunkingResult> DocumentProcessor::process_documents_with_chunking(const std::vector<std::string>& file_paths) {
    std::vector<chunking::ChunkingResult> results;
    results.reserve(file_paths.size());
    
    for (const auto& file_path : file_paths) {
        results.push_back(process_document_with_chunking(file_path));
    }
    
    return results;
}

DocumentResult DocumentProcessor::process_document(const std::string& file_path) {
    auto result = process_single_document(file_path);
    
    // If chunking is enabled, add chunking results
    if (enable_chunking_ && chunker_ && result.processing_success) {
        auto chunking_result = process_document_with_chunking(file_path);
        
        result.chunks = chunking_result.chunks;
        result.total_chunks = chunking_result.total_chunks;
        result.successful_chunks = chunking_result.successful_chunks;
        result.avg_chunk_quality = chunking_result.avg_quality_score;
        result.avg_chunk_density = chunking_result.avg_information_density;
    }
    
    return result;
}

DocumentResult DocumentProcessor::process_document_from_memory(const std::string& file_name, const std::vector<uint8_t>& file_data) {
    (void)file_name; // Suppress unused parameter warning
    (void)file_data; // Suppress unused parameter warning
    
    DocumentResult result;
    result.processing_success = false;
    result.error_message = "Memory processing not implemented yet";
    return result;
}

std::vector<DocumentResult> DocumentProcessor::process_documents_parallel(const std::vector<std::string>& file_paths) {
    std::vector<DocumentResult> results;
    results.reserve(file_paths.size());
    
    if (file_paths.empty()) {
        return results;
    }
    
    // For now, process sequentially to avoid memory issues
    // TODO: Re-enable parallel processing once memory issues are resolved
    for (const auto& file_path : file_paths) {
        try {
            auto result = process_document(file_path);
            results.push_back(std::move(result));
            update_stats(results.back());
        } catch (const std::exception& e) {
            // Create error result
            DocumentResult error_result;
            error_result.file_name = file_path;
            error_result.processing_success = false;
            error_result.error_message = std::string("Processing failed: ") + e.what();
            results.push_back(std::move(error_result));
        }
    }
    
    return results;
}

std::vector<DocumentResult> DocumentProcessor::process_documents_batch(const std::vector<std::string>& file_paths) {
    std::vector<DocumentResult> all_results;
    
    // Use optimal batch size for better parallelization
    size_t optimal_batch_size = parallel::OptimizedThreadPool::get_optimal_batch_size();
    
    // Process files in optimal batches
    for (size_t i = 0; i < file_paths.size(); i += optimal_batch_size) {
        size_t batch_end = std::min(i + optimal_batch_size, file_paths.size());
        std::vector<std::string> batch_files(file_paths.begin() + i, file_paths.begin() + batch_end);
        
        // Process this batch in parallel
        auto batch_results = process_documents_parallel(batch_files);
        all_results.insert(all_results.end(), 
                         std::make_move_iterator(batch_results.begin()),
                         std::make_move_iterator(batch_results.end()));
    }
    
    return all_results;
}

std::vector<DocumentResult> DocumentProcessor::process_documents_with_filtering(const std::vector<std::string>& file_paths) {
    // Process all documents first
    auto all_results = process_documents_batch(file_paths);
    
    // Apply filtering
    std::vector<DocumentResult> filtered_results;
    filtered_results.reserve(all_results.size());
    
    for (const auto& result : all_results) {
        if (should_filter_document(result)) {
            filtered_results.push_back(result);
        } else {
            // Update filtered statistics
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.filtered_out++;
        }
    }
    
    return filtered_results;
}

bool DocumentProcessor::is_supported_file_type(const std::string& file_path) const {
    return format_processor_->is_supported_file_type(file_path);
}

std::vector<std::string> DocumentProcessor::get_supported_extensions() const {
    return format_processor_->get_supported_extensions();
}

ProcessingStats DocumentProcessor::get_processing_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

void DocumentProcessor::reset_stats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_ = ProcessingStats{};
}

size_t DocumentProcessor::get_optimal_batch_size() const {
    return parallel::OptimizedThreadPool::get_optimal_batch_size();
}

// Private methods

DocumentResult DocumentProcessor::process_single_document(const std::string& file_path) {
    DocumentResult result;
    result.processing_start = std::chrono::steady_clock::now();
    result.file_name = utils::TextUtils::get_file_name(file_path);
    result.file_extension = utils::TextUtils::get_file_extension(file_path);
    result.processing_success = false;
    
    // Initialize quality scores
    result.content_quality_score = 0.0;
    result.information_density = 0.0;
    result.is_high_quality = false;
    result.quality_reason = "";
    
    try {
        // Pipeline orchestration using modular components
        processing::PipelineStage validation_stage;
        if (!pipeline_->validate_file(file_path, validation_stage)) {
            result.error_message = validation_stage.error_message;
            result.processing_end = std::chrono::steady_clock::now();
            result.processing_time_ms = std::chrono::duration<double, std::milli>(
                result.processing_end - result.processing_start).count();
            update_stats(result);
            return result;
        }
        
        // Extract text based on file type
        std::string text_content;
        processing::PipelineStage extraction_stage;
        if (!pipeline_->extract_text(file_path, extraction_stage, text_content)) {
            result.error_message = extraction_stage.error_message;
            result.processing_end = std::chrono::steady_clock::now();
            result.processing_time_ms = std::chrono::duration<double, std::milli>(
                result.processing_end - result.processing_start).count();
            update_stats(result);
            return result;
        }
        
        // Clean text
        processing::PipelineStage cleaning_stage;
        if (!pipeline_->clean_text(text_content, cleaning_stage)) {
            result.error_message = cleaning_stage.error_message;
            result.processing_end = std::chrono::steady_clock::now();
            result.processing_time_ms = std::chrono::duration<double, std::milli>(
                result.processing_end - result.processing_start).count();
            update_stats(result);
            return result;
        }
        
        // Extract metadata
        processing::PipelineStage metadata_stage;
        pipeline_->extract_metadata(file_path, metadata_stage, result.metadata);
        
        // Quality assessment
        auto quality_metrics = quality_assessor_->assess_quality(text_content);
        result.content_quality_score = quality_metrics.content_quality_score;
        result.information_density = quality_metrics.information_density;
        result.is_high_quality = quality_metrics.is_high_quality;
        result.quality_reason = quality_metrics.quality_reason;
        
        // Set final text content
        result.text_content = text_content;
        result.processing_success = true;
        
    } catch (const std::exception& e) {
        result.error_message = "Processing failed: " + std::string(e.what());
    }
    
    result.processing_end = std::chrono::steady_clock::now();
    result.processing_time_ms = std::chrono::duration<double, std::milli>(
        result.processing_end - result.processing_start).count();
    
    update_stats(result);
    return result;
}

void DocumentProcessor::update_stats(const DocumentResult& result) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    stats_.total_files_processed++;
    if (result.processing_success) {
        stats_.successful_processing++;
        stats_.total_text_extracted += result.text_content.length();
        
        // Update quality statistics
        double total_quality = stats_.avg_content_quality_score * (stats_.successful_processing - 1);
        total_quality += result.content_quality_score;
        stats_.avg_content_quality_score = total_quality / stats_.successful_processing;
        
        // Update format-specific stats
        auto file_type = format_processor_->detect_file_type(result.file_name);
        switch (file_type) {
            case formats::FileType::PDF:
                stats_.pdf_files_processed++;
                break;
            case formats::FileType::PLAIN_TEXT:
                stats_.text_files_processed++;
                break;
            case formats::FileType::HTML:
                stats_.html_files_processed++;
                break;
            default:
                break;
        }
    } else {
        stats_.failed_processing++;
    }
    
    // Update average processing time
    double total_time = stats_.avg_processing_time_ms * (stats_.total_files_processed - 1);
    total_time += result.processing_time_ms;
    stats_.avg_processing_time_ms = total_time / stats_.total_files_processed;
}

bool DocumentProcessor::should_filter_document(const DocumentResult& result) const {
    if (!result.processing_success) {
        return false;
    }
    
    // Create quality metrics for filtering
    quality::QualityMetrics metrics;
    metrics.text_length = result.text_content.length();
    metrics.content_quality_score = result.content_quality_score;
    metrics.information_density = result.information_density;
    metrics.is_high_quality = result.is_high_quality;
    
    return quality_assessor_->filter_document(metrics);
}

} // namespace core
} // namespace r3m 