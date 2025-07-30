#include "r3m/core/document_processor.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>

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
    
    // Initialize statistics
    stats_.total_files_processed = 0;
    stats_.successful_processing = 0;
    stats_.failed_processing = 0;
    stats_.avg_processing_time_ms = 0.0;
    stats_.total_text_extracted = 0;
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
    
    return true;
}

DocumentResult DocumentProcessor::process_document(const std::string& file_path) {
    DocumentResult result;
    result.processing_start = std::chrono::steady_clock::now();
    result.file_name = std::filesystem::path(file_path).filename().string();
    result.file_extension = get_file_extension(file_path);
    result.processing_success = false;
    
    try {
        // Validate file
        if (!validate_file(file_path, result)) {
            return result;
        }
        
        // Extract text
        if (!extract_text(file_path, result)) {
            return result;
        }
        
        // Clean text
        if (!clean_text(result)) {
            return result;
        }
        
        // Extract metadata
        if (!extract_metadata(file_path, result)) {
            return result;
        }
        
        result.processing_success = true;
        
    } catch (const std::exception& e) {
        result.error_message = e.what();
    }
    
    result.processing_end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        result.processing_end - result.processing_start);
    result.processing_time_ms = duration.count() / 1000.0;
    
    // Update statistics
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.total_files_processed++;
        if (result.processing_success) {
            stats_.successful_processing++;
            stats_.total_text_extracted += result.text_content.length();
            
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
        if (stats_.total_files_processed > 0) {
            stats_.avg_processing_time_ms = 
                (stats_.avg_processing_time_ms * (stats_.total_files_processed - 1) + result.processing_time_ms) 
                / stats_.total_files_processed;
        }
    }
    
    return result;
}

DocumentResult DocumentProcessor::process_document_from_memory(const std::string& file_name, const std::vector<uint8_t>& file_data) {
    // Create temporary file and process it
    std::string temp_path = "/tmp/r3m_" + file_name;
    std::ofstream temp_file(temp_path, std::ios::binary);
    temp_file.write(reinterpret_cast<const char*>(file_data.data()), file_data.size());
    temp_file.close();
    
    DocumentResult result = process_document(temp_path);
    
    // Clean up temp file
    std::filesystem::remove(temp_path);
    
    return result;
}

std::vector<DocumentResult> DocumentProcessor::process_documents_parallel(const std::vector<std::string>& file_paths) {
    std::vector<DocumentResult> results;
    results.reserve(file_paths.size());
    
    // Simple parallel processing (in real implementation, use thread pool)
    for (const auto& file_path : file_paths) {
        results.push_back(process_document(file_path));
    }
    
    return results;
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
    
    if (!extract_metadata_) {
        return true;
    }
    
    // Basic metadata extraction
    result.metadata["file_size"] = std::to_string(result.file_size);
    result.metadata["file_extension"] = result.file_extension;
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    result.metadata["processed_at"] = std::ctime(&time_t);
    
    return true;
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
    // Simple encoding detection (in real implementation, use chardet or similar)
    return default_encoding_;
}

bool DocumentProcessor::is_text_file(const std::string& file_path) const {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    char buffer[1024];
    file.read(buffer, sizeof(buffer));
    file.close();
    
    // Check if file contains mostly printable characters
    for (int i = 0; i < file.gcount(); ++i) {
        if (buffer[i] < 7 || (buffer[i] > 13 && buffer[i] < 32) || buffer[i] > 126) {
            return false;
        }
    }
    
    return true;
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
    std::string result = text;
    
    // Simple HTML tag removal (in real implementation, use proper HTML parser)
    std::regex html_tag_regex("<[^>]*>");
    result = std::regex_replace(result, html_tag_regex, "");
    
    return result;
}

std::string DocumentProcessor::clean_text_content(const std::string& text) {
    std::string result = text;
    
    // Remove null characters
    result.erase(std::remove(result.begin(), result.end(), '\0'), result.end());
    
    // Remove control characters except newlines and tabs
    result.erase(std::remove_if(result.begin(), result.end(), 
        [](char c) { return c < 32 && c != '\n' && c != '\t' && c != '\r'; }), result.end());
    
    return result;
}

} // namespace core
} // namespace r3m 