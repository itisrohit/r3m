#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <string>
#include <iomanip>
#include <filesystem>
#include "r3m/core/document_processor.hpp"

using namespace r3m::core;

void print_separator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << " " << title << "\n";
    std::cout << std::string(60, '=') << "\n";
}

std::string generate_test_document(size_t size_kb) {
    std::string document;
    size_t target_size = size_kb * 1024;
    
    std::vector<std::string> words = {
        "document", "processing", "system", "performance", "optimization",
        "chunking", "tokenization", "metadata", "analysis", "quality",
        "assessment", "parallel", "threading", "memory", "efficiency",
        "throughput", "latency", "benchmark", "profiling", "monitoring"
    };
    
    while (document.size() < target_size) {
        for (size_t i = 0; i < 10; ++i) {
            if (i > 0) document += " ";
            document += words[i % words.size()];
        }
        document += ". ";
    }
    
    return document.substr(0, target_size);
}

int main() {
    std::cout << "📊 R3M Document Size Benchmark\n";
    std::cout << "================================\n\n";
    
    // Initialize document processor
    auto processor = std::make_unique<DocumentProcessor>();
    
    std::unordered_map<std::string, std::string> config;
    config["document_processing.enable_chunking"] = "true";
    config["document_processing.max_workers"] = "4";
    config["document_processing.batch_size"] = "16";  // Optimal batch size
    
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
    
    if (!processor->initialize(config)) {
        std::cerr << "❌ Failed to initialize DocumentProcessor\n";
        return 1;
    }
    
    std::cout << "✅ DocumentProcessor initialized successfully\n";
    
    // Test different document sizes
    std::vector<size_t> sizes = {1, 10, 50, 100, 500}; // KB
    
    print_separator("DOCUMENT SIZE BENCHMARKING");
    
    std::cout << "📊 Processing different document sizes...\n\n";
    
    for (size_t size : sizes) {
        std::string filename = "data/size_test_" + std::to_string(size) + "kb.txt";
        
        // Create test file
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "❌ Failed to create " << filename << "\n";
            continue;
        }
        
        std::string content = generate_test_document(size);
        file << content;
        file.close();
        
        std::cout << "🔍 Testing " << size << "KB document (" << content.size() << " bytes)\n";
        
        // Benchmark sequential processing
        auto start = std::chrono::high_resolution_clock::now();
        auto result = processor->process_document(filename);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "  📊 Results:\n";
        std::cout << "    Processing Time: " << duration.count() << " ms\n";
        std::cout << "    Success: " << (result.processing_success ? "✅" : "❌") << "\n";
        std::cout << "    Text Extracted: " << result.text_content.length() << " bytes\n";
        std::cout << "    Quality Score: " << result.content_quality_score << "\n";
        std::cout << "    Chunks Generated: " << result.total_chunks << "\n";
        
        if (result.processing_success) {
            double throughput = (content.size() / 1024.0) / (duration.count() / 1000.0);
            std::cout << "    Throughput: " << std::fixed << std::setprecision(2) 
                      << throughput << " KB/s\n";
        }
        
        std::cout << "\n";
        
        // Clean up
        std::filesystem::remove(filename);
    }
    
    // Test parallel processing with small files
    print_separator("PARALLEL PROCESSING BENCHMARK");
    
    std::vector<std::string> parallel_files;
    for (int i = 0; i < 8; ++i) {
        std::string filename = "data/parallel_size_test_" + std::to_string(i) + ".txt";
        std::ofstream file(filename);
        
        if (file.is_open()) {
            std::string content = generate_test_document(10); // 10KB each
            file << content;
            file.close();
            parallel_files.push_back(filename);
        }
    }
    
    std::cout << "🔍 Testing parallel processing with " << parallel_files.size() << " files\n";
    
    // Sequential baseline
    auto seq_start = std::chrono::high_resolution_clock::now();
    for (const auto& file : parallel_files) {
        processor->process_document(file);
    }
    auto seq_end = std::chrono::high_resolution_clock::now();
    auto seq_duration = std::chrono::duration_cast<std::chrono::milliseconds>(seq_end - seq_start);
    
    // Parallel processing
    auto par_start = std::chrono::high_resolution_clock::now();
    auto par_results = processor->process_documents_parallel(parallel_files);
    auto par_end = std::chrono::high_resolution_clock::now();
    auto par_duration = std::chrono::duration_cast<std::chrono::milliseconds>(par_end - par_start);
    
    std::cout << "  📊 Results:\n";
    std::cout << "    Sequential Time: " << seq_duration.count() << " ms\n";
    std::cout << "    Parallel Time: " << par_duration.count() << " ms\n";
    
    double speedup = static_cast<double>(seq_duration.count()) / par_duration.count();
    double efficiency = (speedup / 4.0) * 100.0; // 4 worker threads
    
    std::cout << "    Speedup: " << std::fixed << std::setprecision(2) << speedup << "x\n";
    std::cout << "    Efficiency: " << efficiency << "%\n";
    std::cout << "    Time Saved: " << (seq_duration.count() - par_duration.count()) << " ms\n";
    
    // Clean up
    for (const auto& file : parallel_files) {
        std::filesystem::remove(file);
    }
    
    print_separator("BENCHMARK SUMMARY");
    
    std::cout << "✅ Document size benchmarking completed successfully!\n";
    std::cout << "📊 Performance data collected for different document sizes\n";
    std::cout << "🚀 Parallel processing efficiency: " << efficiency << "%\n";
    
    return 0;
} 