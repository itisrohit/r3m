#include "r3m/server/http_server.hpp"
#include "r3m/utils/text_utils.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

namespace r3m {
namespace server {

HttpServer::HttpServer() {
}

bool HttpServer::initialize(const std::unordered_map<std::string, std::string>& config) {
    // Load configuration
    config_.load_from_config(config);
    
    if (!config_.validate()) {
        std::cerr << "Invalid server configuration" << std::endl;
        return false;
    }
    
    // Initialize document processor
    processor_ = std::make_shared<core::DocumentProcessor>();
    if (!processor_->initialize(config)) {
        std::cerr << "Failed to initialize document processor" << std::endl;
        return false;
    }
    
    // Initialize config manager (optional - use passed config if file not found)
    config_manager_ = std::make_unique<core::ConfigManager>();
    bool config_loaded = config_manager_->load_config("configs/dev/config.yaml");
    if (!config_loaded) {
        std::cout << "Info: Using passed configuration (config file not found)" << std::endl;
        // Use the passed config instead
        config_manager_->load_from_map(config);
    }
    
    // Initialize modules
    api_routes_ = std::make_unique<api::Routes>(processor_);
    job_manager_ = std::make_unique<api::JobManager>();
    
    // Create upload directory
    if (!create_upload_directory()) {
        std::cerr << "Failed to create upload directory" << std::endl;
        return false;
    }
    
    return true;
}

bool HttpServer::start() {
#ifdef R3M_HTTP_ENABLED
    if (!setup_routes()) {
        return false;
    }
    
    if (!setup_cors()) {
        return false;
    }
    
    if (!setup_middleware()) {
        return false;
    }
    
    // Start the server
    app_->port(config_.port)
         .multithreaded()
         .run();
    
    return true;
#else
    std::cerr << "HTTP server not enabled (Crow library not found)" << std::endl;
    return false;
#endif
}

void HttpServer::stop() {
#ifdef R3M_HTTP_ENABLED
    if (app_) {
        app_->stop();
    }
#endif
}

bool HttpServer::is_running() const {
#ifdef R3M_HTTP_ENABLED
    return app_ != nullptr;
#else
    return false;
#endif
}

bool HttpServer::setup_routes() {
#ifdef R3M_HTTP_ENABLED
    app_ = std::make_unique<crow::SimpleApp>();
    
    // Health check endpoint
    CROW_ROUTE((*app_), "/health")
    .methods("GET"_method)
    ([this]() {
        return api_routes_->handle_health_check();
    });
    
    // Process single document
    CROW_ROUTE((*app_), "/process")
    .methods("POST"_method)
    ([this](const crow::request& req) {
        return api_routes_->handle_process_document(req);
    });
    
    // Process batch of documents
    CROW_ROUTE((*app_), "/batch")
    .methods("POST"_method)
    ([this](const crow::request& req) {
        return api_routes_->handle_process_batch(req);
    });
    
    // Chunk single document
    CROW_ROUTE((*app_), "/chunk")
    .methods("POST"_method)
    ([this](const crow::request& req) {
        return api_routes_->handle_chunk_document(req);
    });
    
    // Get job status
    CROW_ROUTE((*app_), "/job/<string>")
    .methods("GET"_method)
    ([this](const std::string& job_id) {
        return api_routes_->handle_job_status(job_id);
    });
    
    // Get system info
    CROW_ROUTE((*app_), "/info")
    .methods("GET"_method)
    ([this]() {
        return api_routes_->handle_system_info();
    });
    
    // Get performance metrics
    CROW_ROUTE((*app_), "/metrics")
    .methods("GET"_method)
    ([this]() {
        return api_routes_->handle_metrics();
    });
    
    return true;
#else
    return false;
#endif
}

bool HttpServer::setup_cors() {
#ifdef R3M_HTTP_ENABLED
    if (config_.enable_cors) {
        // CORS is handled automatically by Crow
        return true;
    }
    return true;
#else
    return false;
#endif
}

bool HttpServer::setup_middleware() {
#ifdef R3M_HTTP_ENABLED
    // Add any middleware here (logging, authentication, etc.)
    return true;
#else
    return false;
#endif
}

bool HttpServer::create_upload_directory() {
    try {
        std::filesystem::create_directories(config_.upload_dir);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create upload directory: " << e.what() << std::endl;
        return false;
    }
}

bool HttpServer::save_uploaded_file(const std::string& filename, const std::string& content) {
    std::string file_path = config_.upload_dir + "/" + filename;
    std::ofstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    file << content;
    file.close();
    return true;
}

std::string HttpServer::get_file_extension(const std::string& filename) {
    size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos) {
        return "";
    }
    return filename.substr(pos);
}

bool HttpServer::is_supported_file_type(const std::string& extension) {
    static const std::vector<std::string> supported_extensions = {
        ".txt", ".md", ".json", ".csv", ".html", ".htm", ".pdf"
    };
    
    std::string lower_ext = extension;
    std::transform(lower_ext.begin(), lower_ext.end(), lower_ext.begin(), ::tolower);
    
    return std::find(supported_extensions.begin(), supported_extensions.end(), lower_ext) != supported_extensions.end();
}

} // namespace server
} // namespace r3m 