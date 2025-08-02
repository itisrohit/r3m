#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <memory>
#include "r3m/formats/processor.hpp"

namespace r3m {
namespace processing {

struct PipelineStage {
    std::string name;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    bool success;
    std::string error_message;
};

struct PipelineMetrics {
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

class PipelineOrchestrator {
public:
    PipelineOrchestrator();
    ~PipelineOrchestrator() = default;

    bool initialize(const std::unordered_map<std::string, std::string>& config);
    
    // Pipeline coordination
    bool validate_file(const std::string& file_path, PipelineStage& stage);
    bool extract_text(const std::string& file_path, PipelineStage& stage, std::string& text_content);
    bool clean_text(std::string& text_content, PipelineStage& stage);
    bool extract_metadata(const std::string& file_path, PipelineStage& stage, std::unordered_map<std::string, std::string>& metadata);
    
    // Metrics and statistics
    PipelineMetrics get_metrics() const;
    void update_metrics(const PipelineStage& stage, bool success, size_t text_length = 0);
    void reset_metrics();

private:
    std::unordered_map<std::string, std::string> config_;
    mutable std::mutex metrics_mutex_;
    PipelineMetrics metrics_;
    
    // Format processor for text extraction
    std::unique_ptr<formats::FormatProcessor> format_processor_;
    
    // Configuration settings
    size_t max_file_size_;
    size_t max_text_length_;
    bool encoding_detection_;
    std::string default_encoding_;
    bool remove_html_tags_;
    bool normalize_whitespace_;
    bool extract_metadata_;
};

} // namespace processing
} // namespace r3m 