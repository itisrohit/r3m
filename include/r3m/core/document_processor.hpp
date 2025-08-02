#pragma once

#include "r3m/processing/pipeline.hpp"
#include "r3m/quality/assessor.hpp"
#include "r3m/parallel/optimized_thread_pool.hpp"
#include "r3m/formats/processor.hpp"
#include "r3m/utils/text_utils.hpp"
#include "r3m/chunking/advanced_chunker.hpp"
#include "r3m/chunking/tokenizer.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <memory>
#include <mutex>
#include <atomic>

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
    
    // Chunking results
    std::vector<chunking::DocumentChunk> chunks;
    size_t total_chunks = 0;
    size_t successful_chunks = 0;
    double avg_chunk_quality = 0.0;
    double avg_chunk_density = 0.0;
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
    
    // Parallel processing performance metrics
    size_t total_tasks_processed = 0;
    size_t work_steals = 0;
    double avg_task_time_ms = 0.0;
    double parallel_efficiency = 0.0;
    size_t optimal_batch_size = 0;
};

/**
 * @brief Optimized Document Processor with Advanced Parallel Processing
 * 
 * Features:
 * - Single pool strategy to avoid dual thread pool conflicts
 * - Thread affinity for better cache locality
 * - Work stealing for improved load balancing
 * - Memory pooling to reduce allocation overhead
 * - Optimal batch sizing based on CPU cores
 * - Performance monitoring and statistics
 */
class DocumentProcessor {
public:
    DocumentProcessor();
    ~DocumentProcessor() = default;
    
    // Initialize with configuration
    bool initialize(const std::unordered_map<std::string, std::string>& config);
    
    // Core processing methods
    DocumentResult process_document(const std::string& file_path);
    DocumentResult process_document_from_memory(const std::string& file_name, const std::vector<uint8_t>& file_data);
    
    // Parallel processing methods
    std::vector<DocumentResult> process_documents_parallel(const std::vector<std::string>& file_paths);
    std::vector<DocumentResult> process_documents_batch(const std::vector<std::string>& file_paths);
    std::vector<DocumentResult> process_documents_with_filtering(const std::vector<std::string>& file_paths);
    
    // Chunking methods
    chunking::ChunkingResult process_document_with_chunking(const std::string& file_path);
    std::vector<chunking::ChunkingResult> process_documents_with_chunking(const std::vector<std::string>& file_paths);
    
    // Utility methods
    bool is_supported_file_type(const std::string& file_path) const;
    std::vector<std::string> get_supported_extensions() const;
    ProcessingStats get_processing_stats() const;
    void reset_stats();
    
    // Performance monitoring
    double get_parallel_efficiency() const;
    size_t get_optimal_batch_size() const;
    void print_performance_metrics() const;
    
    // Public utility methods
    bool should_filter_document(const DocumentResult& result) const;
    ProcessingStats get_statistics() const { return stats_; }

private:
    // Modular components
    std::unique_ptr<processing::PipelineOrchestrator> pipeline_;
    std::unique_ptr<quality::QualityAssessor> quality_assessor_;
    std::unique_ptr<formats::FormatProcessor> format_processor_;
    std::unique_ptr<parallel::OptimizedThreadPool> thread_pool_;
    
    // Configuration
    std::unordered_map<std::string, std::string> config_;
    size_t batch_size_;
    size_t max_workers_;
    bool enable_chunking_;
    bool initialized_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    ProcessingStats stats_;
    
    // Chunking components
    std::shared_ptr<chunking::Tokenizer> tokenizer_;
    std::unique_ptr<chunking::AdvancedChunker> chunker_;
    
    // Private methods
    void initialize_chunking_components();
    chunking::AdvancedChunker::Config create_chunker_config();
    chunking::AdvancedChunker::DocumentInfo create_document_info(const std::string& file_path, const std::string& text_content, const std::unordered_map<std::string, std::string>& metadata);
    
    DocumentResult process_single_document(const std::string& file_path);
    void update_stats(const DocumentResult& result);
    
    // Performance optimization methods
    void optimize_parallel_processing();
    void calculate_parallel_efficiency();
};

} // namespace core
} // namespace r3m 