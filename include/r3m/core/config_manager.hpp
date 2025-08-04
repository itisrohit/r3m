#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <optional>
#include <vector>

namespace r3m {
namespace core {

/**
 * @brief Configuration Manager - Handles all configuration loading and management
 *
 * Supports:
 * - YAML configuration files
 * - Environment variable overrides
 * - Default values
 * - Configuration validation
 */
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    // Disable copy constructor and assignment
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    /**
     * @brief Load configuration from file
     * @param config_path Path to YAML configuration file
     * @return true if load successful
     */
    bool load_config(const std::string& config_path);

    /**
     * @brief Load configuration from environment variables
     * @return true if load successful
     */
    bool load_from_environment();

    /**
     * @brief Load configuration from map
     * @param config_map Map of configuration key-value pairs
     * @return true if load successful
     */
    bool load_from_map(const std::unordered_map<std::string, std::string>& config_map);

    /**
     * @brief Validate configuration
     * @return true if configuration is valid
     */
    bool validate_config() const;

    /**
     * @brief Get string value from configuration
     * @param key Configuration key (dot notation: "server.port")
     * @param default_value Default value if key not found
     * @return String value
     */
    std::string get_string(const std::string& key, const std::string& default_value = "") const;

    /**
     * @brief Get integer value from configuration
     * @param key Configuration key
     * @param default_value Default value if key not found
     * @return Integer value
     */
    int get_int(const std::string& key, int default_value = 0) const;

    /**
     * @brief Get double value from configuration
     * @param key Configuration key
     * @param default_value Default value if key not found
     * @return Double value
     */
    double get_double(const std::string& key, double default_value = 0.0) const;

    /**
     * @brief Get boolean value from configuration
     * @param key Configuration key
     * @param default_value Default value if key not found
     * @return Boolean value
     */
    bool get_bool(const std::string& key, bool default_value = false) const;

    /**
     * @brief Get string array from configuration
     * @param key Configuration key
     * @return Vector of strings
     */
    std::vector<std::string> get_string_array(const std::string& key) const;

    /**
     * @brief Set configuration value
     * @param key Configuration key
     * @param value String value
     */
    void set_value(const std::string& key, const std::string& value);

    /**
     * @brief Check if configuration key exists
     * @param key Configuration key
     * @return true if key exists
     */
    bool has_key(const std::string& key) const;

    /**
     * @brief Get all configuration keys
     * @return Vector of configuration keys
     */
    std::vector<std::string> get_keys() const;

    /**
     * @brief Get configuration as string (for debugging)
     * @return Configuration as string
     */
    std::string to_string() const;

    /**
     * @brief Get all configuration as map
     * @return All configuration key-value pairs
     */
    std::unordered_map<std::string, std::string> get_all_config() const;

    // Server configuration
    struct ServerConfig {
        int port = 7860;
        std::string host = "0.0.0.0";
        int threads = 4;
        bool enable_cors = true;
        std::vector<std::string> cors_origins = {"*"};
    };
    ServerConfig get_server_config() const;

    // Model configuration
    struct ModelConfig {
        std::string path;
        int batch_size = 32;
        int max_length = 256;
        bool normalize = true;
        int cache_size = 1000;
    };
    ModelConfig get_embedding_model_config() const;

    // Logging configuration
    struct LoggingConfig {
        std::string level = "info";
        std::string format = "text";
        std::string output = "/app/data/logs/r3m.log";
        std::string max_size = "10MB";
        int max_files = 5;
    };
    LoggingConfig get_logging_config() const;

    // Storage configuration
    struct StorageConfig {
        std::string data_path = "/app/data";
        std::string cache_path = "/app/data/cache";
        std::string log_path = "/app/data/logs";
        std::string temp_path = "/tmp/r3m";
    };
    StorageConfig get_storage_config() const;

    // Performance configuration
    struct PerformanceConfig {
        std::string max_memory = "2GB";
        std::string cache_memory = "512MB";
        int max_threads = 4;
        int batch_timeout = 10;
    };
    PerformanceConfig get_performance_config() const;

private:
    // Configuration storage
    std::unordered_map<std::string, std::string> config_values_;
    std::string config_file_path_;

    // Private methods
    bool parse_yaml_file(const std::string& file_path);
    bool parse_environment_variables();
    std::string get_env_value(const std::string& key) const;
    std::vector<std::string> split_key(const std::string& key) const;
    std::string build_nested_key(const std::vector<std::string>& parts) const;
    bool validate_server_config() const;
    bool validate_model_config() const;
    bool validate_logging_config() const;
    bool validate_storage_config() const;
    bool validate_performance_config() const;
};

} // namespace core
} // namespace r3m 