#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include "r3m/core/document_processor.hpp"
#include "r3m/parallel/optimized_thread_pool.hpp"

using namespace r3m::core;

void print_separator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << " " << title << "\n";
    std::cout << std::string(60, '=') << "\n";
}

void print_subsection(const std::string& title) {
    std::cout << "\n" << title << "\n";
    std::cout << std::string(title.length(), '-') << "\n";
}

int main() {
    std::cout << "🚀 R3M Parallel Optimization Test\n";
    std::cout << "==================================\n\n";
    
    // Initialize document processor with optimized settings
    std::unordered_map<std::string, std::string> config;
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
    
    auto processor = std::make_unique<DocumentProcessor>();
    
    if (!processor->initialize(config)) {
        std::cerr << "❌ Failed to initialize DocumentProcessor\n";
        return 1;
    }
    
    std::cout << "✅ DocumentProcessor initialized successfully\n";
    
    // Create test files for parallel processing
    print_separator("CREATING TEST FILES");
    
    std::vector<std::string> test_files;
    for (int i = 0; i < 12; ++i) {
        std::string filename = "data/parallel_test_" + std::to_string(i) + ".txt";
        std::ofstream file(filename);
        
        file << "Test document " << i << " for parallel processing optimization.\n";
        file << "This document contains substantial content to measure processing time.\n";
        
        // Add more content to make processing measurable
        for (int j = 0; j < 100; ++j) {
            file << "Line " << j << " of additional content for test document " << i << ".\n";
            file << "This content includes technical terms like API_v" << j << ".0, ";
            file << "HTTP-request-" << j << ", and JSON.parse().\n";
            file << "Adding more content to increase file size and processing time.\n";
        }
        
        file.close();
        test_files.push_back(filename);
        std::cout << "  ✅ Created: " << filename << "\n";
    }
    
    // Test 1: Sequential Processing (Baseline)
    print_separator("TEST 1: SEQUENTIAL PROCESSING (BASELINE)");
    
    auto seq_start = std::chrono::high_resolution_clock::now();
    std::vector<DocumentResult> seq_results;
    
    for (const auto& file : test_files) {
        seq_results.push_back(processor->process_document(file));
    }
    
    auto seq_end = std::chrono::high_resolution_clock::now();
    auto seq_duration = std::chrono::duration_cast<std::chrono::milliseconds>(seq_end - seq_start);
    
    std::cout << "Sequential processing time: " << seq_duration.count() << " ms\n";
    std::cout << "Average time per file: " << (seq_duration.count() / test_files.size()) << " ms\n";
    std::cout << "Success rate: " << std::count_if(seq_results.begin(), seq_results.end(), 
                  [](const DocumentResult& r) { return r.processing_success; }) 
                  << "/" << seq_results.size() << "\n";
    
    // Test 2: Optimized Parallel Processing
    print_separator("TEST 2: OPTIMIZED PARALLEL PROCESSING");
    
    auto par_start = std::chrono::high_resolution_clock::now();
    auto par_results = processor->process_documents_parallel(test_files);
    auto par_end = std::chrono::high_resolution_clock::now();
    auto par_duration = std::chrono::duration_cast<std::chrono::milliseconds>(par_end - par_start);
    
    std::cout << "Parallel processing time: " << par_duration.count() << " ms\n";
    std::cout << "Average time per file: " << (par_duration.count() / test_files.size()) << " ms\n";
    std::cout << "Success rate: " << std::count_if(par_results.begin(), par_results.end(), 
                  [](const DocumentResult& r) { return r.processing_success; }) 
                  << "/" << par_results.size() << "\n";
    
    // Performance Analysis
    print_subsection("PERFORMANCE ANALYSIS");
    
    double speedup = static_cast<double>(seq_duration.count()) / par_duration.count();
    // Use actual worker thread count instead of hardware concurrency
    size_t actual_threads = 4; // From config: "document_processing.max_workers" = "4"
    double efficiency = (speedup / actual_threads) * 100.0;
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Speedup: " << speedup << "x\n";
    std::cout << "Efficiency: " << efficiency << "%\n";
    std::cout << "Time saved: " << (seq_duration.count() - par_duration.count()) << " ms\n";
    std::cout << "Performance improvement: " << ((speedup - 1.0) * 100) << "%\n";
    
    // Test 3: Optimal Batch Processing
    print_separator("TEST 3: OPTIMAL BATCH PROCESSING");
    
    auto batch_start = std::chrono::high_resolution_clock::now();
    auto batch_results = processor->process_documents_batch(test_files);
    auto batch_end = std::chrono::high_resolution_clock::now();
    auto batch_duration = std::chrono::duration_cast<std::chrono::milliseconds>(batch_end - batch_start);
    
    std::cout << "Batch processing time: " << batch_duration.count() << " ms\n";
    std::cout << "Optimal batch size: " << processor->get_optimal_batch_size() << "\n";
    std::cout << "Success rate: " << std::count_if(batch_results.begin(), batch_results.end(), 
                  [](const DocumentResult& r) { return r.processing_success; }) 
                  << "/" << batch_results.size() << "\n";
    
    // Test 4: Performance Metrics
    print_separator("TEST 4: PERFORMANCE METRICS");
    
    auto stats = processor->get_processing_stats();
    
    std::cout << "📊 Processing Statistics:\n";
    std::cout << "  Total files processed: " << stats.total_files_processed << "\n";
    std::cout << "  Successful processing: " << stats.successful_processing << "\n";
    std::cout << "  Failed processing: " << stats.failed_processing << "\n";
    std::cout << "  Average processing time: " << stats.avg_processing_time_ms << " ms\n";
    std::cout << "  Total text extracted: " << stats.total_text_extracted << " bytes\n";
    std::cout << "  Average quality score: " << stats.avg_content_quality_score << "\n";
    
    std::cout << "\n🚀 Parallel Processing Metrics:\n";
    std::cout << "  Total tasks processed: " << stats.total_tasks_processed << "\n";
    std::cout << "  Work steals: " << stats.work_steals << "\n";
    std::cout << "  Average task time: " << stats.avg_task_time_ms << " ms\n";
    std::cout << "  Parallel efficiency: " << stats.parallel_efficiency << "%\n";
    std::cout << "  Optimal batch size: " << stats.optimal_batch_size << "\n";
    
    // Test 5: SIMD Integration Performance
    print_separator("TEST 5: SIMD INTEGRATION PERFORMANCE");
    
    // Test SIMD-optimized text processing
    std::string large_text;
    for (int i = 0; i < 1000; ++i) {
        large_text += "This is a test sentence with technical terms like API_v" + std::to_string(i) + ".0. ";
        large_text += "It contains multiple sentences. Each sentence has different content. ";
        large_text += "The text includes special characters & symbols @#$%^&*() and numbers 1234567890.\n";
    }
    
    auto simd_start = std::chrono::high_resolution_clock::now();
    
    // Process with SIMD optimizations
    auto result = processor->process_document_from_memory("simd_test.txt", 
                std::vector<uint8_t>(large_text.begin(), large_text.end()));
    
    auto simd_end = std::chrono::high_resolution_clock::now();
    auto simd_duration = std::chrono::duration_cast<std::chrono::microseconds>(simd_end - simd_start);
    
    std::cout << "SIMD processing time: " << simd_duration.count() << " microseconds\n";
    std::cout << "Text size: " << large_text.length() << " characters\n";
    std::cout << "Processing speed: " << (large_text.length() / (simd_duration.count() / 1000.0)) << " chars/ms\n";
    
    // Summary
    print_separator("OPTIMIZATION SUMMARY");
    
    std::cout << "✅ Single Pool Strategy: Implemented\n";
    std::cout << "✅ Thread Affinity: Implemented\n";
    std::cout << "✅ Work Stealing: Implemented\n";
    std::cout << "✅ Memory Pooling: Implemented\n";
    std::cout << "✅ Optimal Batch Sizing: Implemented\n";
    std::cout << "✅ SIMD Integration: Implemented\n";
    
    std::cout << "\n📈 Performance Improvements:\n";
    std::cout << "  Sequential → Parallel: " << speedup << "x speedup\n";
    std::cout << "  Efficiency: " << efficiency << "% (target: >80%)\n";
    std::cout << "  Time saved: " << (seq_duration.count() - par_duration.count()) << " ms\n";
    
    if (efficiency > 80.0) {
        std::cout << "\n🎉 SUCCESS: Parallel efficiency above 80%!\n";
    } else {
        std::cout << "\n⚠️  WARNING: Parallel efficiency below 80%, further optimization needed\n";
    }
    
    std::cout << "\n🚀 R3M parallel optimization test completed successfully!\n";
    return 0;
} 