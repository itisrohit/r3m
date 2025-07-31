#include "r3m/api/config/config.hpp"
#include <algorithm>
#include <sstream>

namespace r3m {
namespace api {

void Config::load_from_config(const std::unordered_map<std::string, std::string>& config) {
    // Server settings
    if (config.count("server.port")) {
        port = std::stoi(config.at("server.port"));
    }
    if (config.count("server.host")) {
        host = config.at("server.host");
    }
    if (config.count("server.threads")) {
        threads = std::stoi(config.at("server.threads"));
    }
    
    // File handling
    if (config.count("server.upload_dir")) {
        upload_dir = config.at("server.upload_dir");
    }
    if (config.count("server.max_file_size_mb")) {
        max_file_size_mb = std::stoi(config.at("server.max_file_size_mb"));
    }
    
    // CORS settings
    if (config.count("server.enable_cors")) {
        enable_cors = (config.at("server.enable_cors") == "true");
    }
    if (config.count("server.cors_origin")) {
        cors_origin = config.at("server.cors_origin");
    }
    
    // Job settings
    if (config.count("server.max_jobs")) {
        max_jobs = std::stoi(config.at("server.max_jobs"));
    }
    if (config.count("server.job_timeout_seconds")) {
        job_timeout_seconds = std::stoi(config.at("server.job_timeout_seconds"));
    }
    
    // Performance settings
    if (config.count("server.enable_compression")) {
        enable_compression = (config.at("server.enable_compression") == "true");
    }
    if (config.count("server.request_timeout_seconds")) {
        request_timeout_seconds = std::stoi(config.at("server.request_timeout_seconds"));
    }
}

bool Config::validate() const {
    // Basic validation
    if (port <= 0 || port > 65535) {
        return false;
    }
    
    if (threads <= 0 || threads > 64) {
        return false;
    }
    
    if (max_file_size_mb <= 0 || max_file_size_mb > 1000) {
        return false;
    }
    
    if (max_jobs <= 0 || max_jobs > 10000) {
        return false;
    }
    
    if (job_timeout_seconds <= 0 || job_timeout_seconds > 3600) {
        return false;
    }
    
    if (request_timeout_seconds <= 0 || request_timeout_seconds > 300) {
        return false;
    }
    
    return true;
}

std::unordered_map<std::string, std::string> Config::to_string_map() const {
    std::unordered_map<std::string, std::string> result;
    
    result["port"] = std::to_string(port);
    result["host"] = host;
    result["threads"] = std::to_string(threads);
    result["upload_dir"] = upload_dir;
    result["max_file_size_mb"] = std::to_string(max_file_size_mb);
    result["enable_cors"] = enable_cors ? "true" : "false";
    result["cors_origin"] = cors_origin;
    result["max_jobs"] = std::to_string(max_jobs);
    result["job_timeout_seconds"] = std::to_string(job_timeout_seconds);
    result["enable_compression"] = enable_compression ? "true" : "false";
    result["request_timeout_seconds"] = std::to_string(request_timeout_seconds);
    
    return result;
}

} // namespace api
} // namespace r3m 