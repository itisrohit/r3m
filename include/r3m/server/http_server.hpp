#pragma once

#include "r3m/core/document_processor.hpp"
#include "r3m/core/config_manager.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include <chrono>

#ifdef R3M_HTTP_ENABLED
#include <crow.h>
#endif

namespace r3m {
namespace server {

struct ProcessingJob {
    std::string job_id;
    std::string file_path;
    std::chrono::steady_clock::time_point created_at;
    bool completed = false;
    core::DocumentResult result;
    std::string error_message;
};

struct ServerConfig {
    int port = 8080;
    std::string host = "0.0.0.0";
    int threads = 4;
    bool enable_cors = true;
    std::vector<std::string> cors_origins = {"*"};
    std::string upload_dir = "/tmp/r3m/uploads";
    int max_file_size_mb = 100;
    int job_timeout_seconds = 300;
};

class HttpServer {
public:
    HttpServer();
    ~HttpServer() = default;

    bool initialize(const std::unordered_map<std::string, std::string>& config);
    bool start();
    void stop();
    bool is_running() const;

    // Configuration
    ServerConfig get_config() const { return config_; }

private:
    ServerConfig config_;
    std::unique_ptr<core::DocumentProcessor> processor_;
    std::unique_ptr<core::ConfigManager> config_manager_;
    
    // Job management
    std::unordered_map<std::string, ProcessingJob> jobs_;
    std::mutex jobs_mutex_;
    
    // HTTP server (if enabled)
#ifdef R3M_HTTP_ENABLED
    std::unique_ptr<crow::SimpleApp> app_;
#endif

    // Helper methods
    std::string generate_job_id();
    bool setup_routes();
    bool setup_cors();
    std::string create_response(bool success, const std::string& message, const std::string& data = "");
    
    // Route handlers
    crow::response handle_health_check();
    crow::response handle_process_document(const crow::request& req);
    crow::response handle_process_batch(const crow::request& req);
    crow::response handle_job_status(const std::string& job_id);
    crow::response handle_job_result();
    crow::response handle_system_info();
    
    // File handling
    bool save_uploaded_file(const std::string& filename, const std::string& content);
    std::string get_file_extension(const std::string& filename);
    bool is_supported_file_type(const std::string& extension);
};

} // namespace server
} // namespace r3m 