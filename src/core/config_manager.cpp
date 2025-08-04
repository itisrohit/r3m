#include "r3m/core/config_manager.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

namespace r3m {
namespace core {

ConfigManager::ConfigManager() {
    // Initialize with default values
}

ConfigManager::~ConfigManager() = default;

bool ConfigManager::load_config(const std::string& config_path) {
    config_file_path_ = config_path;
    
    // For now, use simple key-value parsing
    // In real implementation, use yaml-cpp
    std::ifstream file(config_path);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Simple key-value parsing
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            if (!key.empty()) {
                config_values_[key] = value;
            }
        }
    }
    
    return true;
}

bool ConfigManager::load_from_environment() {
    // Load from environment variables with R3M_ prefix
    for (const auto& env : {"R3M_SERVER_PORT", "R3M_SERVER_HOST", "R3M_LOG_LEVEL"}) {
        const char* value = std::getenv(env);
        if (value) {
            std::string key = env;
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            config_values_[key] = value;
        }
    }
    
    return true;
}

bool ConfigManager::load_from_map(const std::unordered_map<std::string, std::string>& config_map) {
    // Load configuration from the provided map
    for (const auto& [key, value] : config_map) {
        config_values_[key] = value;
    }
    return true;
}

bool ConfigManager::validate_config() const {
    return validate_server_config() && 
           validate_model_config() && 
           validate_logging_config() && 
           validate_storage_config() && 
           validate_performance_config();
}

std::string ConfigManager::get_string(const std::string& key, const std::string& default_value) const {
    auto it = config_values_.find(key);
    return (it != config_values_.end()) ? it->second : default_value;
}

int ConfigManager::get_int(const std::string& key, int default_value) const {
    auto it = config_values_.find(key);
    if (it != config_values_.end()) {
        try {
            return std::stoi(it->second);
        } catch (const std::exception&) {
            return default_value;
        }
    }
    return default_value;
}

double ConfigManager::get_double(const std::string& key, double default_value) const {
    auto it = config_values_.find(key);
    if (it != config_values_.end()) {
        try {
            return std::stod(it->second);
        } catch (const std::exception&) {
            return default_value;
        }
    }
    return default_value;
}

bool ConfigManager::get_bool(const std::string& key, bool default_value) const {
    auto it = config_values_.find(key);
    if (it != config_values_.end()) {
        std::string value = it->second;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        return (value == "true" || value == "1" || value == "yes");
    }
    return default_value;
}

std::vector<std::string> ConfigManager::get_string_array(const std::string& key) const {
    auto it = config_values_.find(key);
    if (it != config_values_.end()) {
        std::vector<std::string> result;
        std::string value = it->second;
        
        // Simple array parsing (comma-separated)
        size_t pos = 0;
        while ((pos = value.find(',')) != std::string::npos) {
            std::string item = value.substr(0, pos);
            item.erase(0, item.find_first_not_of(" \t"));
            item.erase(item.find_last_not_of(" \t") + 1);
            if (!item.empty()) {
                result.push_back(item);
            }
            value.erase(0, pos + 1);
        }
        
        // Add last item
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        if (!value.empty()) {
            result.push_back(value);
        }
        
        return result;
    }
    return {};
}

void ConfigManager::set_value(const std::string& key, const std::string& value) {
    config_values_[key] = value;
}

bool ConfigManager::has_key(const std::string& key) const {
    return config_values_.find(key) != config_values_.end();
}

std::vector<std::string> ConfigManager::get_keys() const {
    std::vector<std::string> keys;
    keys.reserve(config_values_.size());
    for (const auto& pair : config_values_) {
        keys.push_back(pair.first);
    }
    return keys;
}

std::string ConfigManager::to_string() const {
    std::stringstream ss;
    ss << "Configuration (" << config_values_.size() << " items):" << std::endl;
    for (const auto& pair : config_values_) {
        ss << "  " << pair.first << ": " << pair.second << std::endl;
    }
    return ss.str();
}

std::unordered_map<std::string, std::string> ConfigManager::get_all_config() const {
    return config_values_;
}

ConfigManager::ServerConfig ConfigManager::get_server_config() const {
    ServerConfig config;
    config.port = get_int("server.port", 7860);
    config.host = get_string("server.host", "0.0.0.0");
    config.threads = get_int("server.threads", 4);
    config.enable_cors = get_bool("server.enable_cors", true);
    config.cors_origins = get_string_array("server.cors_origins");
    if (config.cors_origins.empty()) {
        config.cors_origins = {"*"};
    }
    return config;
}

ConfigManager::ModelConfig ConfigManager::get_embedding_model_config() const {
    ModelConfig config;
    config.path = get_string("models.embedding.path", "");
    config.batch_size = get_int("models.embedding.batch_size", 32);
    config.max_length = get_int("models.embedding.max_length", 256);
    config.normalize = get_bool("models.embedding.normalize", true);
    config.cache_size = get_int("models.embedding.cache_size", 1000);
    return config;
}

ConfigManager::LoggingConfig ConfigManager::get_logging_config() const {
    LoggingConfig config;
    config.level = get_string("logging.level", "info");
    config.format = get_string("logging.format", "text");
    config.output = get_string("logging.output", "/app/data/logs/r3m.log");
    config.max_size = get_string("logging.max_size", "10MB");
    config.max_files = get_int("logging.max_files", 5);
    return config;
}

ConfigManager::StorageConfig ConfigManager::get_storage_config() const {
    StorageConfig config;
    config.data_path = get_string("storage.data_path", "/app/data");
    config.cache_path = get_string("storage.cache_path", "/app/data/cache");
    config.log_path = get_string("storage.log_path", "/app/data/logs");
    config.temp_path = get_string("storage.temp_path", "/tmp/r3m");
    return config;
}

ConfigManager::PerformanceConfig ConfigManager::get_performance_config() const {
    PerformanceConfig config;
    config.max_memory = get_string("performance.max_memory", "2GB");
    config.cache_memory = get_string("performance.cache_memory", "512MB");
    config.max_threads = get_int("performance.max_threads", 4);
    config.batch_timeout = get_int("performance.batch_timeout", 10);
    return config;
}

// Private methods

bool ConfigManager::validate_server_config() const {
    auto config = get_server_config();
    return config.port > 0 && config.port <= 65535 && !config.host.empty();
}

bool ConfigManager::validate_model_config() const {
    auto config = get_embedding_model_config();
    return config.batch_size > 0 && config.max_length > 0;
}

bool ConfigManager::validate_logging_config() const {
    auto config = get_logging_config();
    return !config.level.empty() && !config.output.empty();
}

bool ConfigManager::validate_storage_config() const {
    auto config = get_storage_config();
    return !config.data_path.empty() && !config.cache_path.empty();
}

bool ConfigManager::validate_performance_config() const {
    auto config = get_performance_config();
    return config.max_threads > 0 && config.batch_timeout > 0;
}

} // namespace core
} // namespace r3m 