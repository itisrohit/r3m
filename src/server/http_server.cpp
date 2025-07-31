#include "r3m/server/http_server.hpp"
#include "r3m/utils/text_utils.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>
#include <iostream>

#ifdef R3M_HTTP_ENABLED
#include <crow.h>
#endif

#ifdef R3M_JSON_ENABLED
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#else
// Simple JSON fallback when nlohmann/json is not available
namespace json {
    class basic_json {
    public:
        std::string dump() const { return "{}"; }
        void push_back(const basic_json&) {}
        basic_json& operator[](const std::string&) { return *this; }
        basic_json& operator[](int) { return *this; }
        template<typename T>
        basic_json& operator=(const T&) { return *this; }
        bool has(const std::string&) const { return false; }
        std::string s() const { return ""; }
        bool b() const { return false; }
        int i() const { return 0; }
        double d() const { return 0.0; }
    };
    using json = basic_json;
    json array() { return json(); }
    json parse(const std::string&) { return json(); }
}
#endif

namespace r3m {
namespace server {

HttpServer::HttpServer() {
    processor_ = std::make_unique<core::DocumentProcessor>();
    config_manager_ = std::make_unique<core::ConfigManager>();
}

bool HttpServer::initialize(const std::unordered_map<std::string, std::string>& config) {
    // Load server configuration
    auto it = config.find("server.port");
    if (it != config.end()) {
        config_.port = std::stoi(it->second);
    }
    
    it = config.find("server.host");
    if (it != config.end()) {
        config_.host = it->second;
    }
    
    it = config.find("server.threads");
    if (it != config.end()) {
        config_.threads = std::stoi(it->second);
    }
    
    it = config.find("server.upload_dir");
    if (it != config.end()) {
        config_.upload_dir = it->second;
    }
    
    // Initialize document processor
    if (!processor_->initialize(config)) {
        return false;
    }
    
    // Initialize config manager
    if (!config_manager_->load_config("configs/dev/config.yaml")) {
        return false;
    }
    
    // Create upload directory
    std::filesystem::create_directories(config_.upload_dir);
    
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
    
    // Start the server
    app_->port(config_.port)
         .multithreaded()
         .run();
    
    return true;
#else
    // HTTP server not enabled
    std::cout << "⚠️  HTTP server not enabled (Crow library not found)" << std::endl;
    std::cout << "📝 To enable HTTP server, install Crow library" << std::endl;
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

std::string HttpServer::generate_job_id() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* hex_chars = "0123456789abcdef";
    
    std::string job_id;
    for (int i = 0; i < 32; ++i) {
        job_id += hex_chars[dis(gen)];
    }
    return job_id;
}

bool HttpServer::setup_routes() {
#ifdef R3M_HTTP_ENABLED
    app_ = std::make_unique<crow::SimpleApp>();
    
    // Health check endpoint
    CROW_ROUTE((*app_), "/health")
    .methods("GET"_method)
    ([this]() {
        return handle_health_check();
    });
    
    // Process single document
    CROW_ROUTE((*app_), "/process")
    .methods("POST"_method)
    ([this](const crow::request& req) {
        return handle_process_document(req);
    });
    
    // Process batch of documents
    CROW_ROUTE((*app_), "/batch")
    .methods("POST"_method)
    ([this](const crow::request& req) {
        return handle_process_batch(req);
    });
    
    // Get job status
    CROW_ROUTE((*app_), "/job/<string>")
    .methods("GET"_method)
    ([this](const std::string& job_id) {
        return handle_job_status(job_id);
    });
    
    // Get system info
    CROW_ROUTE((*app_), "/info")
    .methods("GET"_method)
    ([this]() {
        return handle_system_info();
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

std::string HttpServer::create_response(bool success, const std::string& message, const std::string& data) {
#ifdef R3M_JSON_ENABLED
    json response;
    response["success"] = success;
    response["message"] = message;
    if (!data.empty()) {
        response["data"] = json::parse(data);
    }
    return response.dump();
#else
    // Simple text response if JSON not available
    if (data.empty()) {
        return "{\"success\":" + std::string(success ? "true" : "false") + 
               ",\"message\":\"" + message + "\"}";
    } else {
        return "{\"success\":" + std::string(success ? "true" : "false") + 
               ",\"message\":\"" + message + "\",\"data\":" + data + "}";
    }
#endif
}

crow::response HttpServer::handle_health_check() {
#ifdef R3M_HTTP_ENABLED
    crow::response res;
    res.code = 200;
    res.set_header("Content-Type", "application/json");
    res.body = create_response(true, "R3M server is healthy");
    return res;
#else
    return crow::response(500, "HTTP server not enabled");
#endif
}

crow::response HttpServer::handle_process_document(const crow::request& req) {
#ifdef R3M_HTTP_ENABLED
    crow::response res;
    res.set_header("Content-Type", "application/json");
    
    try {
        // Parse request body
        auto body = crow::json::load(req.body);
        if (!body) {
            res.code = 400;
            res.body = create_response(false, "Invalid JSON request");
            return res;
        }
        
        // Extract file path or content
        std::string file_path;
        if (body.has("file_path")) {
            file_path = body["file_path"].s();
        } else if (body.has("file_content")) {
            // Save uploaded content to temporary file
            std::string filename = "upload_" + generate_job_id() + ".txt";
            file_path = config_.upload_dir + "/" + filename;
            
            if (!save_uploaded_file(filename, body["file_content"].s())) {
                res.code = 500;
                res.body = create_response(false, "Failed to save uploaded file");
                return res;
            }
        } else {
            res.code = 400;
            res.body = create_response(false, "Missing file_path or file_content");
            return res;
        }
        
        // Process the document
        std::cout << "Processing file: " << file_path << std::endl;
        auto result = processor_->process_document(file_path);
        
        // Create response
        std::string response_data = "{\"job_id\":\"" + generate_job_id() + "\",";
        response_data += "\"file_name\":\"" + result.file_name + "\",";
        response_data += "\"processing_success\":" + std::string(result.processing_success ? "true" : "false") + ",";
        response_data += "\"processing_time_ms\":" + std::to_string(result.processing_time_ms) + ",";
        response_data += "\"text_length\":" + std::to_string(result.text_content.length()) + ",";
        response_data += "\"content_quality_score\":" + std::to_string(result.content_quality_score) + ",";
        response_data += "\"information_density\":" + std::to_string(result.information_density) + ",";
        response_data += "\"is_high_quality\":" + std::string(result.is_high_quality ? "true" : "false") + ",";
        response_data += "\"quality_reason\":\"" + result.quality_reason + "\"}";
        
        res.code = 200;
        res.body = create_response(true, "Document processed successfully", response_data);
        
    } catch (const std::exception& e) {
        res.code = 500;
        res.body = create_response(false, "Processing error: " + std::string(e.what()));
    }
    
    return res;
#else
    // HTTP server not enabled
    return crow::response(500, "HTTP server not enabled");
#endif
}

crow::response HttpServer::handle_process_batch(const crow::request& req) {
#ifdef R3M_HTTP_ENABLED
    crow::response res;
    res.set_header("Content-Type", "application/json");
    
    try {
        // Parse request body
        auto body = crow::json::load(req.body);
        if (!body || !body.has("files")) {
            res.code = 400;
            res.body = create_response(false, "Invalid request: missing files array");
            return res;
        }
        
        std::vector<std::string> file_paths;
        for (const auto& file : body["files"]) {
            file_paths.push_back(file.s());
        }
        
        // Process batch
        auto results = processor_->process_documents_parallel(file_paths);
        
        // Create response
        std::string response_data = "{\"total_files\":" + std::to_string(results.size()) + ",";
        response_data += "\"successful_processing\":" + std::to_string(std::count_if(results.begin(), results.end(),
            [](const core::DocumentResult& r) { return r.processing_success; })) + ",";
        response_data += "\"results\":[";
        
        bool first = true;
        for (const auto& result : results) {
            if (!first) response_data += ",";
            response_data += "{";
            response_data += "\"file_name\":\"" + result.file_name + "\",";
            response_data += "\"processing_success\":" + std::string(result.processing_success ? "true" : "false") + ",";
            response_data += "\"processing_time_ms\":" + std::to_string(result.processing_time_ms) + ",";
            response_data += "\"text_length\":" + std::to_string(result.text_content.length()) + ",";
            response_data += "\"content_quality_score\":" + std::to_string(result.content_quality_score) + ",";
            response_data += "\"information_density\":" + std::to_string(result.information_density) + ",";
            response_data += "\"is_high_quality\":" + std::string(result.is_high_quality ? "true" : "false");
            response_data += "}";
            first = false;
        }
        response_data += "]}";
        
        res.code = 200;
        res.body = create_response(true, "Batch processing completed", response_data);
        
    } catch (const std::exception& e) {
        res.code = 500;
        res.body = create_response(false, "Batch processing error: " + std::string(e.what()));
    }
    
    return res;
#endif
}

crow::response HttpServer::handle_job_status(const std::string& job_id) {
#ifdef R3M_HTTP_ENABLED
    crow::response res;
    res.set_header("Content-Type", "application/json");
    
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    auto it = jobs_.find(job_id);
    
    if (it == jobs_.end()) {
        res.code = 404;
        res.body = create_response(false, "Job not found");
    } else {
        const auto& job = it->second;
        std::string response_data = "{\"job_id\":\"" + job.job_id + "\",";
        response_data += "\"completed\":" + std::string(job.completed ? "true" : "false") + ",";
        response_data += "\"file_path\":\"" + job.file_path + "\"";
        
        if (job.completed) {
            response_data += ",\"processing_success\":" + std::string(job.result.processing_success ? "true" : "false") + ",";
            response_data += "\"processing_time_ms\":" + std::to_string(job.result.processing_time_ms) + ",";
            response_data += "\"text_length\":" + std::to_string(job.result.text_content.length()) + ",";
            response_data += "\"content_quality_score\":" + std::to_string(job.result.content_quality_score) + ",";
            response_data += "\"information_density\":" + std::to_string(job.result.information_density) + ",";
            response_data += "\"is_high_quality\":" + std::string(job.result.is_high_quality ? "true" : "false");
        }
        response_data += "}";
        
        res.code = 200;
        res.body = create_response(true, "Job status retrieved", response_data);
    }
    
    return res;
#endif
}

crow::response HttpServer::handle_system_info() {
#ifdef R3M_HTTP_ENABLED
    crow::response res;
    res.set_header("Content-Type", "application/json");
    
    std::string response_data = "{\"server\":\"R3M Document Processing API\",";
    response_data += "\"version\":\"1.0.0\",";
    response_data += "\"port\":" + std::to_string(config_.port) + ",";
    response_data += "\"host\":\"" + config_.host + "\",";
    response_data += "\"threads\":" + std::to_string(config_.threads) + ",";
    response_data += "\"upload_dir\":\"" + config_.upload_dir + "\",";
    response_data += "\"max_file_size_mb\":" + std::to_string(config_.max_file_size_mb) + ",";
    
    // Get processor statistics
    auto stats = processor_->get_statistics();
    response_data += "\"statistics\":{";
    response_data += "\"total_files_processed\":" + std::to_string(stats.total_files_processed) + ",";
    response_data += "\"successful_processing\":" + std::to_string(stats.successful_processing) + ",";
    response_data += "\"failed_processing\":" + std::to_string(stats.failed_processing) + ",";
    response_data += "\"filtered_out\":" + std::to_string(stats.filtered_out) + ",";
    response_data += "\"avg_processing_time_ms\":" + std::to_string(stats.avg_processing_time_ms) + ",";
    response_data += "\"total_text_extracted\":" + std::to_string(stats.total_text_extracted) + ",";
    response_data += "\"avg_content_quality_score\":" + std::to_string(stats.avg_content_quality_score);
    response_data += "}}";
    
    res.code = 200;
    res.body = create_response(true, "System information retrieved", response_data);
    return res;
#endif
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
    if (pos != std::string::npos) {
        return filename.substr(pos);
    }
    return "";
}

bool HttpServer::is_supported_file_type(const std::string& extension) {
    std::vector<std::string> supported = {".txt", ".md", ".json", ".html", ".pdf"};
    return std::find(supported.begin(), supported.end(), extension) != supported.end();
}

} // namespace server
} // namespace r3m 