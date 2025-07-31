#pragma once

#include "r3m/processing/pipeline.hpp"
#include "r3m/quality/assessor.hpp"
#include "r3m/parallel/thread_pool.hpp"
#include "r3m/formats/processor.hpp"
#include "r3m/utils/text_utils.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <memory>
#include <mutex>

namespace r3m {
namespace core {

struct DocumentResult {
    std::string file_name;
    std::string file_extension;
    size_t file_size = 0;
    std::string text_content;
    std::unordered_map<std::string, std::string> metadata;
    bool processing_success = false;
    std::string error_message;
    std::chrono::steady_clock::time_point processing_start;
    std::chrono::steady_clock::time_point processing_end;
    double processing_time_ms = 0.0;
    
    // Quality assessment results
    double content_quality_score = 0.0;
    double information_density = 0.0;
    bool is_high_quality = false;
    std::string quality_reason;
};

struct ProcessingStats {
    size_t total_files_processed = 0;
    size_t successful_processing = 0;
    size_t failed_processing = 0;
    size_t filtered_out = 0;
    double avg_processing_time_ms = 0.0;
    size_t total_text_extracted = 0;
    double avg_content_quality_score = 0.0;
    size_t pdf_files_processed = 0;
    size_t text_files_processed = 0;
    size_t html_files_processed = 0;
};

class DocumentProcessor {
public:
    DocumentProcessor();
    ~DocumentProcessor() = default;

    bool initialize(const std::unordered_map<std::string, std::string>& config);
    
    // Single document processing
    DocumentResult process_document(const std::string& file_path);
    DocumentResult process_document_from_memory(const std::string& file_name, const std::vector<uint8_t>& file_data);
    
    // Parallel and batch processing
    std::vector<DocumentResult> process_documents_parallel(const std::vector<std::string>& file_paths);
    std::vector<DocumentResult> process_documents_batch(const std::vector<std::string>& file_paths);
    std::vector<DocumentResult> process_documents_with_filtering(const std::vector<std::string>& file_paths);
    
    // File type support
    bool is_supported_file_type(const std::string& file_path) const;
    std::vector<std::string> get_supported_extensions() const;
    
    // Statistics and metrics
    ProcessingStats get_processing_stats() const;
    void reset_stats();
    ProcessingStats get_statistics() const { return stats_; }
    
    // Configuration
    bool is_initialized() const { return initialized_; }
    
    // Public methods for library usage
    bool should_filter_document(const DocumentResult& result) const;

private:
    // Configuration
    std::unordered_map<std::string, std::string> config_;
    bool initialized_ = false;
    
    // Processing components
    std::unique_ptr<processing::PipelineOrchestrator> pipeline_;
    std::unique_ptr<quality::QualityAssessor> quality_assessor_;
    std::unique_ptr<parallel::ThreadPool> thread_pool_;
    std::unique_ptr<formats::FormatProcessor> format_processor_;
    
    // Processing settings
    int batch_size_ = 16;
    int max_workers_ = 4;
    bool enable_parallel_processing_ = true;
    
    // Statistics
    ProcessingStats stats_;
    mutable std::mutex stats_mutex_;
    
    // Internal processing methods
    DocumentResult process_single_document(const std::string& file_path);
    void update_stats(const DocumentResult& result);
};

} // namespace core
} // namespace r3m 