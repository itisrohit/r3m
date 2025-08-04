#pragma once

#include <memory>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>

#include "r3m/core/document_processor.hpp"
#include "r3m/core/pipeline_orchestrator.hpp"

namespace r3m {
namespace core {

/**
 * @brief Engine Status
 */
enum class EngineStatus {
    UNINITIALIZED,
    INITIALIZING,
    RUNNING,
    STOPPING,
    STOPPED,
    ERROR
};

/**
 * @brief Engine Configuration
 */
struct EngineConfig {
    // Server settings
    int port = 7860;
    std::string host = "0.0.0.0";
    int threads = 4;
    
    // Processing settings (core formats only)
    size_t max_workers = 4;
    size_t queue_size = 1000;
    bool enable_parallel_processing = true;
    
    // Performance settings
    size_t max_memory_mb = 2048;
    size_t cache_memory_mb = 512;
    int batch_timeout_seconds = 30;
    
    // Monitoring settings
    bool enable_metrics = true;
    int metrics_interval_seconds = 5;
};

/**
 * @brief Engine Metrics
 */
struct EngineMetrics {
    EngineStatus status;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point last_activity;
    size_t total_requests;
    size_t successful_requests;
    size_t failed_requests;
    double avg_response_time_ms;
    size_t active_connections;
    size_t memory_usage_mb;
    double cpu_usage_percent;
    
    // Document processing metrics
    size_t pdf_documents_processed;
    size_t text_documents_processed;
    size_t html_documents_processed;
};

/**
 * @brief R3M Core Engine - Fast document processing and pipeline orchestration
 * 
 * This is the main orchestrator that:
 * - Manages the core document processing pipeline (PDF, plain text, HTML)
 * - Handles HTTP requests for document processing
 * - Coordinates parallel processing
 * - Monitors performance and resources
 * - Provides health and status information
 * 
 * Focused on speed and efficiency in C++
 */
class Engine {
public:
    Engine();
    ~Engine();

    // Disable copy constructor and assignment
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    /**
     * @brief Initialize the engine with configuration
     * @param config Engine configuration
     * @return true if initialization successful
     */
    bool initialize(const EngineConfig& config);

    /**
     * @brief Start the engine
     * @return true if start successful
     */
    bool start();

    /**
     * @brief Stop the engine gracefully
     * @param timeout_seconds Timeout for graceful shutdown
     * @return true if stop successful
     */
    bool stop(int timeout_seconds = 30);

    /**
     * @brief Get current engine status
     * @return Engine status
     */
    EngineStatus get_status() const;

    /**
     * @brief Get engine metrics
     * @return Engine metrics
     */
    EngineMetrics get_metrics() const;

    /**
     * @brief Process a single document
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
     * @brief Get supported file types (core formats only)
     * @return Vector of supported file extensions
     */
    std::vector<std::string> get_supported_file_types() const;

    /**
     * @brief Get pipeline information
     * @return Vector of pipeline stage names
     */
    std::vector<std::string> get_pipeline_stages() const;

    /**
     * @brief Get processing statistics
     * @return Processing statistics from document processor
     */
    DocumentProcessor::ProcessingStats get_processing_stats() const;

    /**
     * @brief Get pipeline metrics
     * @return Pipeline metrics from orchestrator
     */
    PipelineOrchestrator::PipelineMetrics get_pipeline_metrics() const;

    /**
     * @brief Health check
     * @return true if engine is healthy
     */
    bool health_check() const;

    /**
     * @brief Reset all metrics
     */
    void reset_metrics();

private:
    // Core components
    std::unique_ptr<DocumentProcessor> document_processor_;
    std::unique_ptr<PipelineOrchestrator> pipeline_orchestrator_;
    
    // Configuration
    EngineConfig config_;
    
    // State management
    std::atomic<EngineStatus> status_{EngineStatus::UNINITIALIZED};
    std::atomic<bool> running_{false};
    
    // Metrics
    mutable std::mutex metrics_mutex_;
    EngineMetrics metrics_;
    
    // Monitoring thread
    std::thread monitoring_thread_;
    std::atomic<bool> monitoring_running_{false};
    
    // Private methods
    bool initialize_pipeline();
    bool initialize_http_server();
    void monitoring_loop();
    void update_metrics();
    void cleanup_resources();
    bool validate_configuration() const;
};

} // namespace core
} // namespace r3m 