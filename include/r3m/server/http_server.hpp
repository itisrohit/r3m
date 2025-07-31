#pragma once

#include "r3m/core/document_processor.hpp"
#include "r3m/core/config_manager.hpp"
#include "r3m/api/config/config.hpp"
#include "r3m/api/routes/routes.hpp"
#include "r3m/api/jobs/job_manager.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include <chrono>

#ifdef R3M_HTTP_ENABLED
#include <crow.h>
#endif

namespace r3m {
namespace server {

class HttpServer {
public:
    HttpServer();
    ~HttpServer() = default;

    bool initialize(const std::unordered_map<std::string, std::string>& config);
    bool start();
    void stop();
    bool is_running() const;

    // Configuration
    api::Config get_config() const { return config_; }

private:
    // Server components
    api::Config config_;
    std::shared_ptr<core::DocumentProcessor> processor_;
    std::unique_ptr<core::ConfigManager> config_manager_;
    std::unique_ptr<api::Routes> api_routes_;
    std::unique_ptr<api::JobManager> job_manager_;
    
    // HTTP server (if enabled)
#ifdef R3M_HTTP_ENABLED
    std::unique_ptr<crow::SimpleApp> app_;
#endif

    // Setup methods
    bool setup_routes();
    bool setup_cors();
    bool setup_middleware();
    
    // File handling
    bool save_uploaded_file(const std::string& filename, const std::string& content);
    std::string get_file_extension(const std::string& filename);
    bool is_supported_file_type(const std::string& extension);
    bool create_upload_directory();
};

} // namespace server
} // namespace r3m 