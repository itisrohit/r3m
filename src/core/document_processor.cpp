#include "r3m/core/document_processor.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <thread>
#include <future>
#include <execution>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <cmath>
#include <set>

// PDF processing with poppler-cpp
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>

// HTML processing with gumbo-parser
#include <gumbo.h>

namespace r3m {
namespace core {

DocumentProcessor::DocumentProcessor() {
    // Initialize default configuration
    max_file_size_ = 100 * 1024 * 1024;  // 100MB
    max_text_length_ = 1000000;  // 1M characters
    encoding_detection_ = true;
    default_encoding_ = "utf-8";
    remove_html_tags_ = true;
    normalize_whitespace_ = true;
    extract_metadata_ = true;
    
    // Initialize batch processing settings
    batch_size_ = 16;  // Default batch size (from Onyx's INDEX_BATCH_SIZE)
    max_workers_ = std::thread::hardware_concurrency();
    if (max_workers_ == 0) max_workers_ = 4;  // Fallback
    
    // Initialize quality filtering settings
    enable_quality_filtering_ = true;
    min_content_quality_score_ = 0.3;  // Minimum quality score (0.0-1.0)
    min_information_density_ = 0.1;    // Minimum information density
    min_content_length_ = 50;          // Minimum content length
    max_content_length_ = 1000000;     // Maximum content length
    filter_empty_documents_ = true;
    filter_low_quality_documents_ = true;
    
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

DocumentProcessor::~DocumentProcessor() = default;

bool DocumentProcessor::initialize(const std::unordered_map<std::string, std::string>& config) {
    config_ = config;
    
    // Load supported extensions from config
    plain_text_extensions_ = {".txt", ".md", ".mdx", ".conf", ".log", ".json", ".csv", ".tsv", ".xml", ".yml", ".yaml"};
    pdf_extensions_ = {".pdf"};
    html_extensions_ = {".html", ".htm"};
    
    // Load processing settings from config
    auto it = config_.find("document_processing.max_file_size");
    if (it != config_.end()) {
        std::string size_str = it->second;
        if (size_str.find("MB") != std::string::npos) {
            size_t mb = std::stoul(size_str.substr(0, size_str.find("MB")));
            max_file_size_ = mb * 1024 * 1024;
        }
    }
    
    it = config_.find("document_processing.max_text_length");
    if (it != config_.end()) {
        max_text_length_ = std::stoul(it->second);
    }
    
    it = config_.find("document_processing.text_processing.encoding_detection");
    if (it != config_.end()) {
        encoding_detection_ = (it->second == "true");
    }
    
    it = config_.find("document_processing.text_processing.default_encoding");
    if (it != config_.end()) {
        default_encoding_ = it->second;
    }
    
    it = config_.find("document_processing.text_processing.remove_html_tags");
    if (it != config_.end()) {
        remove_html_tags_ = (it->second == "true");
    }
    
    it = config_.find("document_processing.text_processing.normalize_whitespace");
    if (it != config_.end()) {
        normalize_whitespace_ = (it->second == "true");
    }
    
    it = config_.find("document_processing.text_processing.extract_metadata");
    if (it != config_.end()) {
        extract_metadata_ = (it->second == "true");
    }
    
    // Load batch processing settings
    it = config_.find("document_processing.batch_size");
    if (it != config_.end()) {
        batch_size_ = std::stoul(it->second);
    }
    
    it = config_.find("document_processing.max_workers");
    if (it != config_.end()) {
        max_workers_ = std::stoul(it->second);
    }
    
    // Load quality filtering settings
    it = config_.find("document_processing.quality_filtering.enabled");
    if (it != config_.end()) {
        enable_quality_filtering_ = (it->second == "true");
    }
    
    it = config_.find("document_processing.quality_filtering.min_content_quality_score");
    if (it != config_.end()) {
        min_content_quality_score_ = std::stod(it->second);
    }
    
    it = config_.find("document_processing.quality_filtering.min_information_density");
    if (it != config_.end()) {
        min_information_density_ = std::stod(it->second);
    }
    
    it = config_.find("document_processing.quality_filtering.min_content_length");
    if (it != config_.end()) {
        min_content_length_ = std::stoul(it->second);
    }
    
    it = config_.find("document_processing.quality_filtering.max_content_length");
    if (it != config_.end()) {
        max_content_length_ = std::stoul(it->second);
    }
    
    it = config_.find("document_processing.quality_filtering.filter_empty_documents");
    if (it != config_.end()) {
        filter_empty_documents_ = (it->second == "true");
    }
    
    it = config_.find("document_processing.quality_filtering.filter_low_quality_documents");
    if (it != config_.end()) {
        filter_low_quality_documents_ = (it->second == "true");
    }
    
    return true;
}

DocumentResult DocumentProcessor::process_document(const std::string& file_path) {
    DocumentResult result;
    result.processing_start = std::chrono::steady_clock::now();
    result.file_name = std::filesystem::path(file_path).filename().string();
    result.file_extension = get_file_extension(file_path);
    result.processing_success = false;
    
    // Initialize quality scores
    result.content_quality_score = 0.0;
    result.information_density = 0.0;
    result.is_high_quality = false;
    result.quality_reason = "";
    
    try {
        // Pipeline orchestration (following Onyx's pattern)
        if (!validate_file(file_path, result)) {
            return result;
        }
        
        if (!extract_text(file_path, result)) {
            return result;
        }
        
        if (!clean_text(result)) {
            return result;
        }
        
        if (extract_metadata_) {
            extract_metadata(file_path, result);
        }
        
        // Quality assessment (new advanced feature)
        if (enable_quality_filtering_) {
            assess_quality(result);
        }
        
        result.processing_success = true;
        
    } catch (const std::exception& e) {
        result.error_message = "Processing failed: " + std::string(e.what());
    }
    
    result.processing_end = std::chrono::steady_clock::now();
    result.processing_time_ms = std::chrono::duration<double, std::milli>(
        result.processing_end - result.processing_start).count();
    
    // Update statistics
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.total_files_processed++;
        if (result.processing_success) {
            stats_.successful_processing++;
            stats_.total_text_extracted += result.text_content.length();
            
            // Update quality statistics
            if (enable_quality_filtering_) {
                double total_quality = stats_.avg_content_quality_score * (stats_.successful_processing - 1);
                total_quality += result.content_quality_score;
                stats_.avg_content_quality_score = total_quality / stats_.successful_processing;
            }
            
            // Update format-specific stats
            FileType file_type = detect_file_type(file_path);
            switch (file_type) {
                case FileType::PDF:
                    stats_.pdf_files_processed++;
                    break;
                case FileType::PLAIN_TEXT:
                    stats_.text_files_processed++;
                    break;
                case FileType::HTML:
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
    
    // Implement proper parallel processing with thread pool (following Onyx's pattern)
    if (file_paths.empty()) {
        return results;
    }
    
    // Use std::execution::par for parallel processing (C++17)
    std::vector<std::future<DocumentResult>> futures;
    futures.reserve(file_paths.size());
    
    // Create thread pool with max_workers
    std::vector<std::thread> workers;
    std::mutex results_mutex;
    std::condition_variable cv;
    std::queue<std::function<void()>> tasks;
    bool shutdown = false;
    
    // Start worker threads
    for (size_t i = 0; i < max_workers_; ++i) {
        workers.emplace_back([&]() {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(results_mutex);
                    cv.wait(lock, [&]() { return !tasks.empty() || shutdown; });
                    if (shutdown && tasks.empty()) {
                        break;
                    }
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
    
    // Submit tasks to thread pool
    for (const auto& file_path : file_paths) {
        std::unique_lock<std::mutex> lock(results_mutex);
        tasks.emplace([this, file_path, &results, &results_mutex]() {
            DocumentResult result = process_document(file_path);
            {
                std::lock_guard<std::mutex> lock(results_mutex);
                results.push_back(std::move(result));
            }
        });
        cv.notify_one();
    }
    
    // Shutdown workers
    {
        std::unique_lock<std::mutex> lock(results_mutex);
        shutdown = true;
        cv.notify_all();
    }
    
    // Wait for all workers to finish
    for (auto& worker : workers) {
        worker.join();
    }
    
    return results;
}

std::vector<DocumentResult> DocumentProcessor::process_documents_batch(const std::vector<std::string>& file_paths) {
    std::vector<DocumentResult> all_results;
    
    // Process files in batches (following Onyx's batch processing pattern)
    for (size_t i = 0; i < file_paths.size(); i += batch_size_) {
        size_t batch_end = std::min(i + batch_size_, file_paths.size());
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
    
    // Apply filtering (following Onyx's filtering pattern)
    std::vector<DocumentResult> filtered_results;
    filtered_results.reserve(all_results.size());
    
    for (const auto& result : all_results) {
        if (filter_document(result)) {
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
    std::string extension = get_file_extension(file_path);
    
    return std::find(plain_text_extensions_.begin(), plain_text_extensions_.end(), extension) != plain_text_extensions_.end() ||
           std::find(pdf_extensions_.begin(), pdf_extensions_.end(), extension) != pdf_extensions_.end() ||
           std::find(html_extensions_.begin(), html_extensions_.end(), extension) != html_extensions_.end();
}

std::vector<std::string> DocumentProcessor::get_supported_extensions() const {
    std::vector<std::string> all_extensions;
    all_extensions.insert(all_extensions.end(), plain_text_extensions_.begin(), plain_text_extensions_.end());
    all_extensions.insert(all_extensions.end(), pdf_extensions_.begin(), pdf_extensions_.end());
    all_extensions.insert(all_extensions.end(), html_extensions_.begin(), html_extensions_.end());
    return all_extensions;
}

DocumentProcessor::ProcessingStats DocumentProcessor::get_processing_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

// Private methods

bool DocumentProcessor::validate_file(const std::string& file_path, DocumentResult& result) {
    if (!std::filesystem::exists(file_path)) {
        result.error_message = "File does not exist: " + file_path;
        return false;
    }
    
    result.file_size = std::filesystem::file_size(file_path);
    if (result.file_size > max_file_size_) {
        result.error_message = "File too large: " + std::to_string(result.file_size) + " bytes";
        return false;
    }
    
    if (!is_supported_file_type(file_path)) {
        result.error_message = "Unsupported file type: " + get_file_extension(file_path);
        return false;
    }
    
    return true;
}

bool DocumentProcessor::extract_text(const std::string& file_path, DocumentResult& result) {
    FileType file_type = detect_file_type(file_path);
    
    switch (file_type) {
        case FileType::PLAIN_TEXT:
            result.text_content = process_plain_text(file_path);
            break;
        case FileType::PDF:
            result.text_content = process_pdf(file_path);
            break;
        case FileType::HTML:
            result.text_content = process_html(file_path);
            break;
        default:
            result.error_message = "Unsupported file type";
            return false;
    }
    
    if (result.text_content.length() > max_text_length_) {
        result.text_content = result.text_content.substr(0, max_text_length_);
    }
    
    return true;
}

bool DocumentProcessor::clean_text(DocumentResult& result) {
    if (remove_html_tags_) {
        result.text_content = remove_html_tags(result.text_content);
    }
    
    if (normalize_whitespace_) {
        result.text_content = normalize_whitespace(result.text_content);
    }
    
    result.text_content = clean_text_content(result.text_content);
    
    return true;
}

bool DocumentProcessor::extract_metadata(const std::string& file_path, DocumentResult& result) {
    (void)file_path; // Suppress unused parameter warning
    
    // Extract basic metadata
    result.metadata["file_size"] = std::to_string(result.file_size);
    result.metadata["processing_time_ms"] = std::to_string(result.processing_time_ms);
    result.metadata["text_length"] = std::to_string(result.text_content.length());
    
    return true;
}

bool DocumentProcessor::assess_quality(DocumentResult& result) {
    // Calculate quality scores
    result.content_quality_score = calculate_content_quality_score(result.text_content);
    result.information_density = calculate_information_density(result.text_content);
    result.is_high_quality = is_high_quality_content(result);
    
    // Set quality reason
    if (result.is_high_quality) {
        result.quality_reason = "High quality content";
    } else if (result.text_content.length() < min_content_length_) {
        result.quality_reason = "Content too short";
    } else if (result.content_quality_score < min_content_quality_score_) {
        result.quality_reason = "Low content quality score";
    } else if (result.information_density < min_information_density_) {
        result.quality_reason = "Low information density";
    } else {
        result.quality_reason = "Quality assessment failed";
    }
    
    return true;
}

bool DocumentProcessor::filter_document(const DocumentResult& result) const {
    if (!enable_quality_filtering_) {
        return true; // No filtering
    }
    
    // Check if document should be filtered out
    if (filter_empty_documents_ && result.text_content.empty()) {
        return false;
    }
    
    if (result.text_content.length() < min_content_length_) {
        return false;
    }
    
    if (result.text_content.length() > max_content_length_) {
        return false;
    }
    
    if (filter_low_quality_documents_ && !result.is_high_quality) {
        return false;
    }
    
    return true;
}

double DocumentProcessor::calculate_content_quality_score(const std::string& text) {
    if (text.empty()) {
        return 0.0;
    }
    
    // Simple quality scoring based on content characteristics
    double score = 0.0;
    
    // Length factor (0-0.3)
    double length_factor = std::min(1.0, static_cast<double>(text.length()) / 1000.0);
    score += length_factor * 0.3;
    
    // Word diversity factor (0-0.3)
    std::set<std::string> unique_words;
    std::stringstream ss(text);
    std::string word;
    while (ss >> word) {
        // Simple word cleaning
        word.erase(std::remove_if(word.begin(), word.end(), [](char c) { 
            return !std::isalnum(c); 
        }), word.end());
        if (!word.empty()) {
            unique_words.insert(word);
        }
    }
    
    double word_diversity = static_cast<double>(unique_words.size()) / std::max(1.0, static_cast<double>(text.length() / 5.0));
    score += std::min(1.0, word_diversity) * 0.3;
    
    // Sentence structure factor (0-0.2)
    size_t sentence_count = std::count(text.begin(), text.end(), '.') + 
                           std::count(text.begin(), text.end(), '!') + 
                           std::count(text.begin(), text.end(), '?');
    double sentence_factor = std::min(1.0, static_cast<double>(sentence_count) / 10.0);
    score += sentence_factor * 0.2;
    
    // Information density factor (0-0.2)
    double info_density = calculate_information_density(text);
    score += info_density * 0.2;
    
    return std::min(1.0, std::max(0.0, score));
}

double DocumentProcessor::calculate_information_density(const std::string& text) {
    if (text.empty()) {
        return 0.0;
    }
    
    // Calculate information density based on content characteristics
    double density = 0.0;
    
    // Unique word ratio
    std::set<std::string> unique_words;
    std::stringstream ss(text);
    std::string word;
    while (ss >> word) {
        word.erase(std::remove_if(word.begin(), word.end(), [](char c) { 
            return !std::isalnum(c); 
        }), word.end());
        if (!word.empty()) {
            unique_words.insert(word);
        }
    }
    
    double unique_word_ratio = static_cast<double>(unique_words.size()) / std::max(1.0, static_cast<double>(text.length() / 5.0));
    density += unique_word_ratio * 0.4;
    
    // Technical term density (words with numbers, special characters)
    size_t technical_terms = 0;
    std::stringstream ss2(text);
    while (ss2 >> word) {
        if (std::any_of(word.begin(), word.end(), ::isdigit) ||
            std::any_of(word.begin(), word.end(), [](char c) { return c == '_' || c == '-' || c == '.'; })) {
            technical_terms++;
        }
    }
    
    double technical_density = static_cast<double>(technical_terms) / std::max(1.0, static_cast<double>(text.length() / 10.0));
    density += technical_density * 0.3;
    
    // Sentence complexity (average sentence length)
    size_t sentences = std::count(text.begin(), text.end(), '.') + 
                      std::count(text.begin(), text.end(), '!') + 
                      std::count(text.begin(), text.end(), '?');
    if (sentences > 0) {
        double avg_sentence_length = static_cast<double>(text.length()) / sentences;
        double complexity_factor = std::min(1.0, avg_sentence_length / 100.0);
        density += complexity_factor * 0.3;
    }
    
    return std::min(1.0, std::max(0.0, density));
}

bool DocumentProcessor::is_high_quality_content(const DocumentResult& result) const {
    return result.content_quality_score >= min_content_quality_score_ &&
           result.information_density >= min_information_density_ &&
           result.text_content.length() >= min_content_length_ &&
           result.text_content.length() <= max_content_length_;
}

std::string DocumentProcessor::process_plain_text(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + file_path);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string DocumentProcessor::process_pdf(const std::string& file_path) {
    try {
        // Load PDF document
        std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(file_path));
        if (!doc) {
            throw std::runtime_error("Failed to load PDF document");
        }
        
        std::string text_content;
        int num_pages = doc->pages();
        
        // Extract text from each page
        for (int i = 0; i < num_pages; ++i) {
            std::unique_ptr<poppler::page> page(doc->create_page(i));
            if (page) {
                std::string page_text = page->text().to_latin1();
                if (!page_text.empty()) {
                    text_content += page_text + "\n\n";
                }
            }
        }
        
        return text_content;
        
    } catch (const std::exception& e) {
        throw std::runtime_error("PDF processing failed: " + std::string(e.what()));
    }
}

// Helper function to extract text from HTML nodes
void extract_text_from_node(GumboNode* node, std::string& text) {
    if (node->type == GUMBO_NODE_TEXT) {
        text += node->v.text.text;
    } else if (node->type == GUMBO_NODE_ELEMENT) {
        // Skip script and style tags
        if (node->v.element.tag != GUMBO_TAG_SCRIPT && 
            node->v.element.tag != GUMBO_TAG_STYLE) {
            GumboVector* children = &node->v.element.children;
            for (unsigned int i = 0; i < children->length; ++i) {
                extract_text_from_node(static_cast<GumboNode*>(children->data[i]), text);
            }
        }
    }
}

std::string DocumentProcessor::process_html(const std::string& file_path) {
    try {
        // Read HTML file
        std::ifstream file(file_path);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open HTML file: " + file_path);
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string html_content = buffer.str();
        
        // Parse HTML with gumbo
        GumboOutput* output = gumbo_parse(html_content.c_str());
        if (!output) {
            throw std::runtime_error("Failed to parse HTML");
        }
        
        // Extract text from the parsed HTML
        std::string text_content;
        extract_text_from_node(output->root, text_content);
        
        // Clean up gumbo output
        gumbo_destroy_output(&kGumboDefaultOptions, output);
        
        return text_content;
        
    } catch (const std::exception& e) {
        throw std::runtime_error("HTML processing failed: " + std::string(e.what()));
    }
}

DocumentProcessor::FileType DocumentProcessor::detect_file_type(const std::string& file_path) const {
    std::string extension = get_file_extension(file_path);
    
    if (std::find(plain_text_extensions_.begin(), plain_text_extensions_.end(), extension) != plain_text_extensions_.end()) {
        return FileType::PLAIN_TEXT;
    } else if (std::find(pdf_extensions_.begin(), pdf_extensions_.end(), extension) != pdf_extensions_.end()) {
        return FileType::PDF;
    } else if (std::find(html_extensions_.begin(), html_extensions_.end(), extension) != html_extensions_.end()) {
        return FileType::HTML;
    }
    
    return FileType::UNSUPPORTED;
}

std::string DocumentProcessor::get_file_extension(const std::string& file_path) const {
    std::filesystem::path path(file_path);
    return path.extension().string();
}

std::string DocumentProcessor::detect_encoding(const std::string& file_path) {
    (void)file_path; // Suppress unused parameter warning
    return default_encoding_;
}

bool DocumentProcessor::is_text_file(const std::string& file_path) const {
    (void)file_path; // Suppress unused parameter warning
    return true; // Simplified for now
}

std::string DocumentProcessor::normalize_whitespace(const std::string& text) {
    std::string result = text;
    
    // Replace multiple whitespace with single space
    std::regex whitespace_regex("\\s+");
    result = std::regex_replace(result, whitespace_regex, " ");
    
    // Trim leading and trailing whitespace
    result.erase(0, result.find_first_not_of(" \t\n\r"));
    result.erase(result.find_last_not_of(" \t\n\r") + 1);
    
    return result;
}

std::string DocumentProcessor::remove_html_tags(const std::string& text) {
    std::regex html_tag_regex("<[^>]*>");
    return std::regex_replace(text, html_tag_regex, "");
}

std::string DocumentProcessor::clean_text_content(const std::string& text) {
    std::string result = text;
    
    // Remove null characters
    result.erase(std::remove(result.begin(), result.end(), '\0'), result.end());
    
    // Remove other control characters except newlines and tabs
    result.erase(std::remove_if(result.begin(), result.end(),
        [](char c) { return c < 32 && c != '\n' && c != '\t' && c != '\r'; }), result.end());
    
    return result;
}

} // namespace core
} // namespace r3m 