#include "r3m/processing/pipeline.hpp"
#include "r3m/formats/processor.hpp"
#include "r3m/utils/text_utils.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace r3m {
namespace processing {

PipelineOrchestrator::PipelineOrchestrator() {
    // Initialize with default values (will be overridden by config)
    max_file_size_ = 100 * 1024 * 1024; // 100 MB default
    max_text_length_ = 10 * 1024 * 1024; // 10 MB default
    encoding_detection_ = true;
    default_encoding_ = "utf-8";
    remove_html_tags_ = true;
    normalize_whitespace_ = true;
    extract_metadata_ = true;
}

bool PipelineOrchestrator::initialize(const std::unordered_map<std::string, std::string>& config) {
    config_ = config;
    
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

bool PipelineOrchestrator::validate_file(const std::string& file_path, PipelineStage& stage) {
    stage.name = "file_validation";
    stage.start_time = std::chrono::steady_clock::now();
    stage.success = false;
    
    if (!std::filesystem::exists(file_path)) {
        stage.error_message = "File does not exist: " + file_path;
        stage.end_time = std::chrono::steady_clock::now();
        return false;
    }
    
    size_t file_size = std::filesystem::file_size(file_path);
    if (file_size > max_file_size_) {
        stage.error_message = "File too large: " + std::to_string(file_size) + " bytes";
        stage.end_time = std::chrono::steady_clock::now();
        return false;
    }
    
    // Check if file type is supported (basic check)
    std::string extension = utils::TextUtils::get_file_extension(file_path);
    if (extension.empty()) {
        stage.error_message = "No file extension found";
        stage.end_time = std::chrono::steady_clock::now();
        return false;
    }
    
    stage.success = true;
    stage.end_time = std::chrono::steady_clock::now();
    return true;
}

bool PipelineOrchestrator::extract_text(const std::string& file_path, PipelineStage& stage, std::string& text_content) {
    stage.name = "text_extraction";
    stage.start_time = std::chrono::steady_clock::now();
    stage.success = false;
    
    try {
        // This will be handled by the FormatProcessor
        // For now, just read the file as text
        std::ifstream file(file_path);
        if (!file.is_open()) {
            stage.error_message = "Cannot open file: " + file_path;
            stage.end_time = std::chrono::steady_clock::now();
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        text_content = buffer.str();
        
        if (text_content.length() > max_text_length_) {
            text_content = text_content.substr(0, max_text_length_);
        }
        
        stage.success = true;
        stage.end_time = std::chrono::steady_clock::now();
        return true;
        
    } catch (const std::exception& e) {
        stage.error_message = "Text extraction failed: " + std::string(e.what());
        stage.end_time = std::chrono::steady_clock::now();
        return false;
    }
}

bool PipelineOrchestrator::clean_text(std::string& text_content, PipelineStage& stage) {
    stage.name = "text_cleaning";
    stage.start_time = std::chrono::steady_clock::now();
    stage.success = false;
    
    try {
        // Remove HTML tags if enabled
        if (remove_html_tags_) {
            text_content = utils::TextUtils::remove_html_tags(text_content);
        }
        
        // Normalize whitespace if enabled
        if (normalize_whitespace_) {
            text_content = utils::TextUtils::normalize_whitespace(text_content);
        }
        
        // Clean text content
        text_content = utils::TextUtils::clean_text_content(text_content);
        
        stage.success = true;
        stage.end_time = std::chrono::steady_clock::now();
        return true;
        
    } catch (const std::exception& e) {
        stage.error_message = "Text cleaning failed: " + std::string(e.what());
        stage.end_time = std::chrono::steady_clock::now();
        return false;
    }
}

bool PipelineOrchestrator::extract_metadata(const std::string& file_path, PipelineStage& stage, std::unordered_map<std::string, std::string>& metadata) {
    stage.name = "metadata_extraction";
    stage.start_time = std::chrono::steady_clock::now();
    stage.success = false;
    
    try {
        // Extract basic metadata
        metadata["file_name"] = utils::TextUtils::get_file_name(file_path);
        metadata["file_extension"] = utils::TextUtils::get_file_extension(file_path);
        metadata["file_size"] = std::to_string(utils::TextUtils::get_file_size(file_path));
        metadata["file_directory"] = utils::TextUtils::get_file_directory(file_path);
        
        stage.success = true;
        stage.end_time = std::chrono::steady_clock::now();
        return true;
        
    } catch (const std::exception& e) {
        stage.error_message = "Metadata extraction failed: " + std::string(e.what());
        stage.end_time = std::chrono::steady_clock::now();
        return false;
    }
}

PipelineMetrics PipelineOrchestrator::get_metrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    return metrics_;
}

void PipelineOrchestrator::update_metrics(const PipelineStage& stage, bool success, size_t text_length) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    metrics_.total_files_processed++;
    
    if (success) {
        metrics_.successful_processing++;
        metrics_.total_text_extracted += text_length;
    } else {
        metrics_.failed_processing++;
    }
    
    // Update average processing time
    auto duration = std::chrono::duration<double, std::milli>(stage.end_time - stage.start_time);
    double processing_time = duration.count();
    
    double total_time = metrics_.avg_processing_time_ms * (metrics_.total_files_processed - 1);
    total_time += processing_time;
    metrics_.avg_processing_time_ms = total_time / metrics_.total_files_processed;
}

void PipelineOrchestrator::reset_metrics() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    metrics_ = PipelineMetrics{};
}

} // namespace processing
} // namespace r3m 