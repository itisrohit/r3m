#pragma once

#include <string>
#include <unordered_map>

namespace r3m {
namespace api {

struct Config {
    // Server settings
    int port = 7860;
    std::string host = "0.0.0.0";
    int threads = 4;
    
    // File handling
    std::string upload_dir = "/tmp/r3m/uploads";
    int max_file_size_mb = 100;
    
    // CORS settings
    bool enable_cors = true;
    std::string cors_origin = "*";
    
    // Job settings
    int max_jobs = 1000;
    int job_timeout_seconds = 300;
    
    // Performance settings
    bool enable_compression = true;
    int request_timeout_seconds = 30;
    
    // Load from configuration map
    void load_from_config(const std::unordered_map<std::string, std::string>& config);
    
    // Validation
    bool validate() const;
    
    // Get as string map for logging
    std::unordered_map<std::string, std::string> to_string_map() const;
};

} // namespace api
} // namespace r3m 