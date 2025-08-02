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
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning
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
        // Default optimized configuration
        config["document_processing.enable_chunking"] = "true";
        config["document_processing.batch_size"] = "16";  // Optimal batch size
        config["document_processing.max_workers"] = "4";
        
        // OPTIMIZED PARALLEL PROCESSING CONFIGURATION
        config["document_processing.enable_optimized_thread_pool"] = "true";
        config["document_processing.enable_thread_affinity"] = "true";
        config["document_processing.enable_work_stealing"] = "true";
        config["document_processing.enable_memory_pooling"] = "true";
        
        // SIMD OPTIMIZATION CONFIGURATION
        config["document_processing.enable_simd_optimizations"] = "true";
        config["document_processing.enable_avx2"] = "true";
        config["document_processing.enable_avx512"] = "true";
        config["document_processing.enable_neon"] = "true";
        
        // CHUNKING CONFIGURATION - OPTIMIZED!
        config["chunking.enable_multipass"] = "true";
        config["chunking.enable_large_chunks"] = "true";
        config["chunking.enable_contextual_rag"] = "true";
        config["chunking.include_metadata"] = "true";
        config["chunking.chunk_token_limit"] = "2048";
        config["chunking.chunk_overlap"] = "0";
        config["chunking.mini_chunk_size"] = "150";
        config["chunking.blurb_size"] = "100";
        config["chunking.large_chunk_ratio"] = "4";
        config["chunking.max_metadata_percentage"] = "0.25";
        config["chunking.contextual_rag_reserved_tokens"] = "512";
        
        // OPTIMIZED TOKEN PROCESSING
        config["chunking.enable_token_caching"] = "true";
        config["chunking.enable_string_view_optimization"] = "true";
        config["chunking.enable_preallocation"] = "true";
        config["chunking.enable_move_semantics"] = "true";
        
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