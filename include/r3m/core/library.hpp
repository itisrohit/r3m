#pragma once

#include "r3m/core/document_processor.hpp"
#include "r3m/core/config_manager.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace r3m {
namespace core {

/**
 * @brief R3M Direct Library Interface - Zero Overhead Processing
 * 
 * This provides direct C++ library access for maximum performance.
 * No HTTP overhead, no serialization, direct function calls.
 */
class Library {
public:
    Library();
    ~Library() = default;

    // Initialization
    bool initialize(const std::string& config_path = "configs/dev/config.yaml");
    bool initialize(const std::unordered_map<std::string, std::string>& config);
    
    // Direct document processing (zero overhead)
    DocumentResult process_document(const std::string& file_path);
    std::vector<DocumentResult> process_documents_parallel(const std::vector<std::string>& file_paths);
    std::vector<DocumentResult> process_documents_batch(const std::vector<std::string>& file_paths);
    
    // Batch processing with quality filtering
    struct BatchResult {
        std::vector<DocumentResult> processed;
        std::vector<DocumentResult> filtered_out;
        size_t total_files;
        size_t successful_processing;
        size_t failed_processing;
        double processing_time_ms;
    };
    
    BatchResult process_batch_with_filtering(const std::vector<std::string>& file_paths);
    
    // Statistics and monitoring
    ProcessingStats get_statistics() const;
    bool is_initialized() const { return initialized_; }
    
    // Configuration
    void update_config(const std::unordered_map<std::string, std::string>& config);
    std::unordered_map<std::string, std::string> get_config() const;

private:
    std::unique_ptr<DocumentProcessor> processor_;
    std::unique_ptr<ConfigManager> config_manager_;
    bool initialized_ = false;
};

} // namespace core
} // namespace r3m 