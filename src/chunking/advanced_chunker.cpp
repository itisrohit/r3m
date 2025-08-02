#include "r3m/chunking/advanced_chunker.hpp"
#include "r3m/utils/text_processing.hpp"
#include <algorithm>
#include <sstream>
#include <chrono>
#include <unordered_set>
#include <unordered_map>

namespace r3m {
namespace chunking {



AdvancedChunker::AdvancedChunker(std::shared_ptr<Tokenizer> tokenizer, const Config& config)
    : tokenizer_(tokenizer), config_(config), 
      token_cache_(std::make_unique<token_management::TokenCache>(tokenizer)),
      optimized_cache_(std::make_unique<token_management::OptimizedTokenCache>(tokenizer)),
      section_processor_(std::make_unique<section_processing::SectionProcessor>(tokenizer)) {
    initialize_components();
}

void AdvancedChunker::initialize_components() {
    // Initialize sentence chunkers
    blurb_splitter_ = std::make_unique<SentenceChunker>(
        tokenizer_, config_.blurb_size, 0, "texts"
    );
    
    chunk_splitter_ = std::make_unique<SentenceChunker>(
        tokenizer_, config_.chunk_token_limit, config_.chunk_overlap, "texts"
    );
    
    if (config_.enable_multipass) {
        mini_chunk_splitter_ = std::make_unique<SentenceChunker>(
            tokenizer_, config_.mini_chunk_size, 0, "texts"
        );
    }
    
    // Initialize advanced components
    if (config_.enable_multipass) {
        multipass_chunker_ = std::make_unique<MultipassChunker>(
            tokenizer_, true, true, config_.mini_chunk_size, config_.large_chunk_ratio, config_.chunk_token_limit
        );
    }
    
    if (config_.enable_contextual_rag) {
        contextual_rag_ = std::make_unique<ContextualRAG>(
            tokenizer_, config_.contextual_rag_reserved_tokens, true, true
        );
    }
}

ChunkingResult AdvancedChunker::process_document(const DocumentInfo& document) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Clear caches for fresh start
    token_cache_->clear();
    optimized_cache_->clear();
    
    ChunkingResult result;
    
    try {
        // Step 2: Manage token allocation
        auto token_result = manage_tokens(document);
        
        // Step 2: Process sections
        auto chunks = process_sections(document, token_result);
        
        // Step 3: Apply quality filtering
        chunks = apply_quality_filtering(chunks);
        
        // Step 4: Apply multipass indexing if enabled
        if (config_.enable_multipass && multipass_chunker_) {
            auto large_chunks = multipass_chunker_->generate_large_chunks(chunks);
            chunks.insert(chunks.end(), large_chunks.begin(), large_chunks.end());
        }
        
        // Step 5: Apply contextual RAG if enabled
        if (config_.enable_contextual_rag && contextual_rag_) {
            contextual_rag_->add_contextual_summaries(chunks);
        }
        
        // Step 6: Calculate final statistics
        result.chunks = chunks;
        result.total_chunks = chunks.size();
        result.successful_chunks = chunks.size();
        result.failed_chunks = 0;
    
    // Calculate quality metrics
    double total_quality = 0.0;
    double total_density = 0.0;
    size_t high_quality_count = 0;
    
        for (const auto& chunk : chunks) {
        total_quality += chunk.quality_score;
        total_density += chunk.information_density;
            if (chunk.is_high_quality) high_quality_count++;
            
            result.total_title_tokens += chunk.title_tokens;
            result.total_metadata_tokens += chunk.metadata_tokens;
            result.total_content_tokens += static_cast<size_t>(tokenizer_->count_tokens(chunk.content));
            result.total_rag_tokens += chunk.contextual_rag_reserved_tokens;
        }
        
        if (!chunks.empty()) {
            result.avg_quality_score = total_quality / chunks.size();
            result.avg_information_density = total_density / chunks.size();
        }
        result.high_quality_chunks = high_quality_count;
        
    } catch (const std::exception& e) {
        result.failed_chunks = 1;
        result.successful_chunks = 0;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    result.processing_time_ms = duration.count() / 1000.0;
    
    return result;
}

std::vector<ChunkingResult> AdvancedChunker::process_documents(const std::vector<DocumentInfo>& documents) {
    std::vector<ChunkingResult> results;
    results.reserve(documents.size());
    
    for (const auto& document : documents) {
        results.push_back(process_document(document));
    }
    
    return results;
}

section_processing::TokenManagementResult AdvancedChunker::manage_tokens(const DocumentInfo& document) {
    section_processing::TokenManagementResult result;
    
    // Step 1: Extract title blurb
    std::string title_blurb = extract_title_blurb(document.title);
    result.title_prefix = title_blurb.empty() ? "" : title_blurb + "\n";
    result.title_tokens = static_cast<int>(tokenizer_->count_tokens(result.title_prefix));
    
    // Step 2: Process metadata
    if (config_.include_metadata && !document.metadata.empty()) {
        auto metadata_result = MetadataProcessor::get_metadata_suffix_for_document_index(
            document.metadata, true
        );
        
        result.metadata_suffix_semantic = metadata_result.first;
        result.metadata_suffix_keyword = metadata_result.second;
        result.metadata_tokens = optimized_cache_->get_token_count(result.metadata_suffix_semantic);
        
        // Check if metadata is too large
        if (MetadataProcessor::is_metadata_too_large(result.metadata_tokens, config_.chunk_token_limit)) {
            result.metadata_suffix_semantic = "";
            result.metadata_tokens = 0;
            result.metadata_too_large = true;
        }
    }
    
    // Step 3: Check if document fits in single chunk
    if (config_.enable_contextual_rag) {
        int doc_tokens = optimized_cache_->get_token_count(document.full_content);
        result.single_chunk_fits = (doc_tokens + result.title_tokens + result.metadata_tokens <= config_.chunk_token_limit);
        
        // Expand context size based on whether chunk context and doc summary are used
        if (!result.single_chunk_fits && !AVERAGE_SUMMARY_EMBEDDINGS) {
            result.contextual_rag_reserved_tokens = config_.contextual_rag_reserved_tokens;
        }
    }
    
    // Step 4: Calculate content token limit
    result.content_token_limit = config_.chunk_token_limit - result.title_tokens - result.metadata_tokens - result.contextual_rag_reserved_tokens;
    
    // Step 5: Check minimum content requirement
    if (result.content_token_limit <= config_.chunk_min_content) {
        // Don't do contextual RAG if not enough content
        result.contextual_rag_reserved_tokens = 0;
        result.content_token_limit = config_.chunk_token_limit - result.title_tokens - result.metadata_tokens;
        
        // If still not enough content, revert to full chunk
        if (result.content_token_limit <= config_.chunk_min_content) {
            result.content_token_limit = config_.chunk_token_limit;
            result.title_prefix = "";
            result.metadata_suffix_semantic = "";
            result.title_tokens = 0;
            result.metadata_tokens = 0;
        }
    }
    
    return result;
}

std::string AdvancedChunker::extract_title_blurb(const std::string& title) {
    if (title.empty()) return "";
    
    // Use blurb splitter to get first sentence(s)
    auto blurbs = blurb_splitter_->chunk(title);
    return blurbs.empty() ? title : blurbs[0];
}

std::vector<DocumentChunk> AdvancedChunker::process_sections(
    const DocumentInfo& document,
    const section_processing::TokenManagementResult& token_result) {
    
    // Use the section processor for advanced section combination logic
    return section_processor_->process_sections_with_combinations(
        document.sections, token_result, document.document_id, 
        document.source_type, document.semantic_identifier
    );
}





std::vector<DocumentChunk> AdvancedChunker::apply_quality_filtering(const std::vector<DocumentChunk>& chunks) {
    std::vector<DocumentChunk> filtered_chunks;
    
    for (const auto& chunk : chunks) {
        // Basic quality checks
        bool is_valid = true;
        
        // Check minimum content length
        if (chunk.content.length() < 50) {
            is_valid = false;
        }
        
        // Check quality score
        if (chunk.quality_score < 0.3) {
            is_valid = false;
        }
        
        // Check information density
        if (chunk.information_density < 0.1) {
            is_valid = false;
        }
        
        if (is_valid) {
            filtered_chunks.push_back(chunk);
        }
    }
    
    return filtered_chunks;
}

void AdvancedChunker::calculate_chunk_quality(DocumentChunk& chunk) {
    // Calculate quality score using the quality calculator
    chunk.quality_score = quality_assessment::QualityCalculator::calculate_quality_score(chunk.content);
    chunk.information_density = quality_assessment::QualityCalculator::calculate_information_density(chunk.content);
    chunk.is_high_quality = chunk.quality_score >= 0.7;
}



void AdvancedChunker::update_config(const Config& config) {
    config_ = config;
    initialize_components();
}

void AdvancedChunker::clear_cache() {
    if (token_cache_) {
        token_cache_->clear();
    }
}

} // namespace chunking
} // namespace r3m 