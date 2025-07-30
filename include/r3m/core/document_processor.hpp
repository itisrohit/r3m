#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <mutex>

namespace r3m {
namespace core {

/**
 * @brief Document Processing Result
 * 
 * Contains extracted text, metadata, and processing information
 */
struct DocumentResult {
    std::string text_content;
    std::string file_name;
    std::string file_extension;
    std::string mime_type;
    size_t file_size;
    std::unordered_map<std::string, std::string> metadata;
    bool processing_success;
    std::string error_message;
    
    // Processing metadata
    std::chrono::steady_clock::time_point processing_start;
    std::chrono::steady_clock::time_point processing_end;
    double processing_time_ms;
};

/**
 * @brief Document Processor - Fast document processing pipeline
 * 
 * Handles core formats:
 * - Plain text files (.txt, .md, .json, .csv, etc.)
 * - PDF files (.pdf)
 * - HTML files (.html)
 * 
 * Focused on speed and efficiency in C++
 */
class DocumentProcessor {
public:
    DocumentProcessor();
    ~DocumentProcessor();

    // Disable copy constructor and assignment
    DocumentProcessor(const DocumentProcessor&) = delete;
    DocumentProcessor& operator=(const DocumentProcessor&) = delete;

    /**
     * @brief Initialize processor with configuration
     * @param config Configuration data
     * @return true if initialization successful
     */
    bool initialize(const std::unordered_map<std::string, std::string>& config);

    /**
     * @brief Process a document from file path
     * @param file_path Path to the document
     * @return DocumentResult with extracted content
     */
    DocumentResult process_document(const std::string& file_path);

    /**
     * @brief Process a document from memory buffer
     * @param file_name Name of the file
     * @param file_data File content as bytes
     * @return DocumentResult with extracted content
     */
    DocumentResult process_document_from_memory(const std::string& file_name, const std::vector<uint8_t>& file_data);

    /**
     * @brief Process multiple documents in parallel
     * @param file_paths Vector of file paths
     * @return Vector of DocumentResult
     */
    std::vector<DocumentResult> process_documents_parallel(const std::vector<std::string>& file_paths);

    /**
     * @brief Check if file type is supported
     * @param file_path Path to file
     * @return true if supported
     */
    bool is_supported_file_type(const std::string& file_path) const;

    /**
     * @brief Get supported file extensions
     * @return Vector of supported extensions
     */
    std::vector<std::string> get_supported_extensions() const;

    /**
     * @brief Get processing statistics
     * @return Processing statistics
     */
    struct ProcessingStats {
        size_t total_files_processed;
        size_t successful_processing;
        size_t failed_processing;
        double avg_processing_time_ms;
        size_t total_text_extracted;
        
        // Format-specific stats
        size_t pdf_files_processed;
        size_t text_files_processed;
        size_t html_files_processed;
    };
    ProcessingStats get_processing_stats() const;

private:
    // File type detection
    enum class FileType {
        PLAIN_TEXT,
        PDF,
        HTML,
        UNSUPPORTED
    };

    // Processing pipeline stages
    bool validate_file(const std::string& file_path, DocumentResult& result);
    bool extract_text(const std::string& file_path, DocumentResult& result);
    bool clean_text(DocumentResult& result);
    bool extract_metadata(const std::string& file_path, DocumentResult& result);

    // Core format processors
    std::string process_plain_text(const std::string& file_path);
    std::string process_pdf(const std::string& file_path);
    std::string process_html(const std::string& file_path);

    // Utility functions
    FileType detect_file_type(const std::string& file_path) const;
    std::string get_file_extension(const std::string& file_path) const;
    std::string detect_encoding(const std::string& file_path);
    bool is_text_file(const std::string& file_path) const;
    std::string normalize_whitespace(const std::string& text);
    std::string remove_html_tags(const std::string& text);
    std::string clean_text_content(const std::string& text);

    // Configuration (loaded from config.yaml)
    std::unordered_map<std::string, std::string> config_;
    std::vector<std::string> plain_text_extensions_;
    std::vector<std::string> pdf_extensions_;
    std::vector<std::string> html_extensions_;
    size_t max_file_size_;
    size_t max_text_length_;
    bool encoding_detection_;
    std::string default_encoding_;
    bool remove_html_tags_;
    bool normalize_whitespace_;
    bool extract_metadata_;

    // Statistics
    mutable std::mutex stats_mutex_;
    ProcessingStats stats_;
};

} // namespace core
} // namespace r3m 