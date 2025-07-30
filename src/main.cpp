#include <iostream>
#include <memory>
#include <signal.h>
#include <unistd.h>

#include "r3m/core/engine.hpp"
#include "r3m/core/config_manager.hpp"

using namespace r3m::core;

// Global engine instance for signal handling
std::unique_ptr<Engine> g_engine = nullptr;

// Signal handler for graceful shutdown
void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Shutting down gracefully..." << std::endl;
    
    if (g_engine) {
        g_engine->stop(30);  // 30 second timeout
    }
    
    exit(0);
}

int main(int argc, char* argv[]) {
    std::cout << "R3M Core Engine - Fast Document Processing Pipeline" << std::endl;
    std::cout << "Version: 1.0.0" << std::endl;
    std::cout << "==================================================" << std::endl;
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    try {
        // Initialize configuration manager
        ConfigManager config_manager;
        
        // Load configuration from file
        std::string config_path = "configs/dev/config.yaml";
        if (argc > 1) {
            config_path = argv[1];
        }
        
        if (!config_manager.load_config(config_path)) {
            std::cerr << "Failed to load configuration from: " << config_path << std::endl;
            return 1;
        }
        
        // Create engine configuration
        EngineConfig engine_config;
        engine_config.port = config_manager.get_int("server.port", 8080);
        engine_config.host = config_manager.get_string("server.host", "0.0.0.0");
        engine_config.threads = config_manager.get_int("server.threads", 4);
        engine_config.max_workers = config_manager.get_int("document_processing.worker_threads", 4);
        engine_config.queue_size = 1000;
        engine_config.enable_parallel_processing = config_manager.get_bool("document_processing.enable_parallel_processing", true);
        engine_config.max_memory_mb = config_manager.get_int("engine.max_memory_mb", 2048);
        engine_config.cache_memory_mb = config_manager.get_int("engine.cache_memory_mb", 512);
        engine_config.batch_timeout_seconds = config_manager.get_int("engine.batch_timeout_seconds", 30);
        engine_config.enable_metrics = config_manager.get_bool("engine.enable_metrics", true);
        engine_config.metrics_interval_seconds = config_manager.get_int("engine.metrics_interval_seconds", 5);
        
        // Create and initialize engine
        g_engine = std::make_unique<Engine>();
        
        if (!g_engine->initialize(engine_config)) {
            std::cerr << "Failed to initialize engine" << std::endl;
            return 1;
        }
        
        std::cout << "Engine initialized successfully" << std::endl;
        std::cout << "Configuration loaded from: " << config_path << std::endl;
        std::cout << "Server will start on: " << engine_config.host << ":" << engine_config.port << std::endl;
        
        // Start the engine
        if (!g_engine->start()) {
            std::cerr << "Failed to start engine" << std::endl;
            return 1;
        }
        
        std::cout << "Engine started successfully" << std::endl;
        std::cout << "Press Ctrl+C to stop" << std::endl;
        
        // Keep the engine running
        while (g_engine->get_status() == EngineStatus::RUNNING) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            // Print status every 10 seconds
            static int counter = 0;
            if (++counter % 10 == 0) {
                auto metrics = g_engine->get_metrics();
                std::cout << "Status: " << static_cast<int>(metrics.status) 
                         << " | Requests: " << metrics.total_requests
                         << " | Memory: " << metrics.memory_usage_mb << "MB" << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Engine stopped" << std::endl;
    return 0;
} 