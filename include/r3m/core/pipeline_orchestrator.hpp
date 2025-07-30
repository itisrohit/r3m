#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <chrono>

#include "r3m/core/document_processor.hpp"

namespace r3m {
namespace core {

/**
 * @brief Pipeline Stage - Represents a processing stage
 */
struct PipelineStage {
    std::string name;
    std::function<bool(DocumentResult&)> processor;
    bool required;
    std::chrono::milliseconds timeout;
};

/**
 * @brief Pipeline Orchestrator - Coordinates core document processing pipeline
 * 
 * Manages:
 * - Pipeline stage execution (file_validation → text_extraction → text_cleaning → metadata_extraction)
 * - Parallel processing for core formats (PDF, plain text, HTML)
 * - Error handling and recovery
 * - Performance monitoring
 * - Resource management
 */
class PipelineOrchestrator {
public:
    PipelineOrchestrator();
    ~PipelineOrchestrator();

    // Disable copy constructor and assignment
    PipelineOrchestrator(const PipelineOrchestrator&) = delete;
    PipelineOrchestrator& operator=(const PipelineOrchestrator&) = delete;

    /**
     * @brief Initialize the pipeline with core processing stages
     * @param stages Vector of pipeline stages
     * @return true if initialization successful
     */
    bool initialize_pipeline(const std::vector<PipelineStage>& stages);

    /**
     * @brief Process a single document through the pipeline
     * @param file_path Path to document
     * @return DocumentResult with processing results
     */
    DocumentResult process_document(const std::string& file_path);

    /**
     * @brief Process multiple documents in parallel
     * @param file_paths Vector of file paths
     * @return Vector of DocumentResult
     */
    std::vector<DocumentResult> process_documents_parallel(const std::vector<std::string>& file_paths);

    /**
     * @brief Add a custom pipeline stage
     * @param stage Pipeline stage to add
     */
    void add_pipeline_stage(const PipelineStage& stage);

    /**
     * @brief Remove a pipeline stage
     * @param stage_name Name of stage to remove
     */
    void remove_pipeline_stage(const std::string& stage_name);

    /**
     * @brief Get pipeline stage information
     * @return Vector of stage names
     */
    std::vector<std::string> get_pipeline_stages() const;

    /**
     * @brief Enable/disable a pipeline stage
     * @param stage_name Name of stage
     * @param enabled Whether to enable the stage
     */
    void set_stage_enabled(const std::string& stage_name, bool enabled);

    /**
     * @brief Get pipeline performance metrics
     * @return Performance metrics
     */
    struct PipelineMetrics {
        size_t total_documents_processed;
        size_t successful_pipelines;
        size_t failed_pipelines;
        double avg_pipeline_time_ms;
        std::unordered_map<std::string, double> stage_avg_times_ms;
        size_t active_workers;
        size_t queue_size;
        
        // Format-specific metrics
        size_t pdf_documents_processed;
        size_t text_documents_processed;
        size_t html_documents_processed;
    };
    PipelineMetrics get_pipeline_metrics() const;

    /**
     * @brief Reset pipeline metrics
     */
    void reset_metrics();

    /**
     * @brief Set pipeline configuration
     * @param max_workers Maximum number of worker threads
     * @param queue_size Size of processing queue
     * @param enable_monitoring Enable performance monitoring
     */
    void set_configuration(size_t max_workers, size_t queue_size, bool enable_monitoring = true);

private:
    // Pipeline stages
    std::vector<PipelineStage> pipeline_stages_;
    std::unordered_map<std::string, bool> stage_enabled_;
    mutable std::mutex stages_mutex_;

    // Processing components
    std::unique_ptr<DocumentProcessor> document_processor_;
    std::vector<std::thread> worker_threads_;
    std::atomic<size_t> active_workers_{0};

    // Queue management
    struct ProcessingTask {
        std::string file_path;
        std::chrono::steady_clock::time_point queued_time;
    };
    std::vector<ProcessingTask> processing_queue_;
    mutable std::mutex queue_mutex_;

    // Performance tracking
    mutable std::mutex metrics_mutex_;
    PipelineMetrics metrics_;
    std::unordered_map<std::string, std::vector<double>> stage_times_;

    // Configuration
    size_t max_workers_;
    size_t queue_size_;
    bool enable_monitoring_;
    std::atomic<bool> running_{false};

    // Private methods
    bool execute_pipeline_stages(DocumentResult& result);
    bool execute_stage(const PipelineStage& stage, DocumentResult& result);
    void worker_thread_function();
    void update_metrics(const std::string& stage_name, double processing_time_ms, bool success);
    void monitor_performance();
    void cleanup_resources();
};

} // namespace core
} // namespace r3m 