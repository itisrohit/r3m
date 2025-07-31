#include "r3m/core/config_manager.hpp"
#include "r3m/server/http_server.hpp"
#include <iostream>
#include <signal.h>

#ifdef R3M_HTTP_ENABLED
#include <crow.h>
#endif

namespace r3m {

std::unique_ptr<server::HttpServer> g_server;

void signal_handler(int signal) {
    if (g_server) {
        std::cout << "\n🛑 Received signal " << signal << ", shutting down R3M server..." << std::endl;
        g_server->stop();
    }
    exit(0);
}

} // namespace r3m

int main(int argc, char* argv[]) {
    std::cout << "🚀 Starting R3M Document Processing Server..." << std::endl;
    
    // Set up signal handlers
    signal(SIGINT, r3m::signal_handler);
    signal(SIGTERM, r3m::signal_handler);
    
    try {
        // Load configuration
        r3m::core::ConfigManager config_manager;
        if (!config_manager.load_config("configs/dev/config.yaml")) {
            std::cerr << "❌ Failed to load configuration" << std::endl;
            return 1;
        }
        
        // Get server configuration
        auto server_config = config_manager.get_server_config();
        
        // Create HTTP server
        r3m::g_server = std::make_unique<r3m::server::HttpServer>();
        
        // Initialize server with configuration
        std::unordered_map<std::string, std::string> config;
        config["server.port"] = std::to_string(server_config.port);
        config["server.host"] = server_config.host;
        config["server.threads"] = std::to_string(server_config.threads);
        config["server.upload_dir"] = "/tmp/r3m/uploads";
        
        // Add document processing configuration
        config["document_processing.max_file_size"] = "100MB";
        config["document_processing.max_text_length"] = "1000000";
        config["document_processing.worker_threads"] = "4";
        config["document_processing.batch_size"] = "16";
        config["document_processing.max_workers"] = "8";
        
        // Add quality filtering configuration
        config["document_processing.quality_filtering.enabled"] = "true";
        config["document_processing.quality_filtering.min_content_quality_score"] = "0.3";
        config["document_processing.quality_filtering.min_information_density"] = "0.1";
        config["document_processing.quality_filtering.min_content_length"] = "50";
        config["document_processing.quality_filtering.max_content_length"] = "1000000";
        
        if (!r3m::g_server->initialize(config)) {
            std::cerr << "❌ Failed to initialize HTTP server" << std::endl;
            return 1;
        }
        
        std::cout << "✅ R3M server initialized successfully" << std::endl;
        std::cout << "🌐 Server will start on http://" << server_config.host << ":" << server_config.port << std::endl;
        std::cout << "📊 Available endpoints:" << std::endl;
        std::cout << "   GET  /health     - Health check" << std::endl;
        std::cout << "   POST /process    - Process single document" << std::endl;
        std::cout << "   POST /batch      - Process batch of documents" << std::endl;
        std::cout << "   GET  /job/{id}   - Get job status" << std::endl;
        std::cout << "   GET  /info       - System information" << std::endl;
        std::cout << "🔄 Press Ctrl+C to stop the server" << std::endl;
        
        // Start the server
        if (!r3m::g_server->start()) {
            std::cerr << "❌ Failed to start HTTP server" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 