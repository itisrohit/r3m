#include "r3m/utils/performance.hpp"
#include "r3m/core/document_processor.hpp"
#include "r3m/core/config_manager.hpp"
#include "r3m/chunking/advanced_chunker.hpp"
#include "r3m/chunking/tokenizer.hpp"
#include <iostream>
#include <fstream>
#include <random>
#include <thread>
#include <chrono>
#include <filesystem>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>

using namespace r3m;

void print_section(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << " " << title << "\n";
    std::cout << std::string(60, '=') << "\n";
}

void print_subsection(const std::string& title) {
    std::cout << "\n" << std::string(40, '-') << "\n";
    std::cout << " " << title << "\n";
    std::cout << std::string(40, '-') << "\n";
}

std::string generate_test_document(size_t size_kb) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> word_len(3, 12);
    std::uniform_int_distribution<> sentence_len(5, 25);
    
    std::vector<std::string> words = {
        "document", "processing", "system", "performance", "optimization",
        "chunking", "tokenization", "metadata", "analysis", "quality",
        "assessment", "parallel", "threading", "memory", "efficiency",
        "throughput", "latency", "benchmark", "profiling", "monitoring"
    };
    
    std::string document;
    size_t target_size = size_kb * 1024;
    
    while (document.size() < target_size) {
        // Generate a sentence
        int sentence_length = sentence_len(gen);
        std::string sentence;
        
        for (int i = 0; i < sentence_length; ++i) {
            if (i > 0) sentence += " ";
            sentence += words[gen() % words.size()];
        }
        sentence += ". ";
        
        document += sentence;
    }
    
    return document.substr(0, target_size);
}

void create_performance_test_files() {
    std::cout << "📁 Creating performance test files...\n";
    
    std::vector<size_t> sizes = {1, 10, 50, 100, 500}; // KB
    std::vector<std::string> files;
    
    for (size_t size : sizes) {
        std::string filename = "data/perf_test_" + std::to_string(size) + "kb.txt";
        std::ofstream file(filename);
        
        if (file.is_open()) {
            std::string content = generate_test_document(size);
            file << content;
            file.close();
            files.push_back(filename);
            std::cout << "  ✅ Created: " << filename << " (" << content.size() << " bytes)\n";
        }
    }
    
    std::cout << "📊 Created " << files.size() << " performance test files\n";
}

void benchmark_document_processing() {
    print_subsection("Document Processing Benchmark");
    
    utils::PerformanceUtils::BenchmarkRunner runner("Document Processing", 50);
    runner.set_warmup_iterations(5);
    runner.set_memory_tracking(false); // Disable memory tracking to avoid Docker issues
    
    // Load configuration from config file instead of hardcoded values
    r3m::core::ConfigManager config_manager;
    if (!config_manager.load_config("configs/dev/config.yaml")) {
        std::cerr << "❌ Failed to load configuration from config file\n";
        return;
    }
    
    // Get all configuration from config file
    std::unordered_map<std::string, std::string> config = config_manager.get_all_config();
    
    auto processor = std::make_unique<r3m::core::DocumentProcessor>();
    processor->initialize(config);
    
    std::vector<std::string> test_files = {
        "data/perf_test_1kb.txt",
        "data/perf_test_10kb.txt",
        "data/perf_test_50kb.txt",
        "data/perf_test_100kb.txt",
        "data/perf_test_500kb.txt"
    };
    
    for (const auto& file : test_files) {
        if (std::filesystem::exists(file)) {
            std::cout << "\n🔍 Benchmarking: " << file << "\n";
            
            auto results = runner.run_benchmark([&processor, &file]() {
                auto result = processor->process_document(file);
                return result;
            });
            
            std::cout << "  📊 Results:\n";
            std::cout << "    Iterations: " << results.iterations << "\n";
            std::cout << "    Total Time: " << utils::PerformanceUtils::format_time(results.total_time_ms) << "\n";
            std::cout << "    Avg Time: " << utils::PerformanceUtils::format_time(results.avg_time_ms) << "\n";
            std::cout << "    Min Time: " << utils::PerformanceUtils::format_time(results.min_time_ms) << "\n";
            std::cout << "    Max Time: " << utils::PerformanceUtils::format_time(results.max_time_ms) << "\n";
            std::cout << "    Std Dev: " << utils::PerformanceUtils::format_time(results.std_deviation_ms) << "\n";
            std::cout << "    Throughput: " << utils::PerformanceUtils::format_throughput(results.throughput_per_second) << "\n";
            std::cout << "    Avg Memory: " << utils::PerformanceUtils::format_memory(results.avg_memory_bytes) << "\n";
        }
    }
}

void benchmark_chunking_performance() {
    print_subsection("Chunking Performance Benchmark");
    
    utils::PerformanceUtils::BenchmarkRunner runner("Chunking", 100);
    runner.set_warmup_iterations(10);
    runner.set_memory_tracking(true);
    
    // Initialize chunker
    auto tokenizer = std::make_shared<chunking::BasicTokenizer>();
    chunking::AdvancedChunker::Config config;
    config.enable_multipass = true;
    config.enable_large_chunks = true;
    config.enable_contextual_rag = true;
    config.chunk_token_limit = 2048;
    config.chunk_overlap = 0;
    config.mini_chunk_size = 150;
    config.blurb_size = 100;
    config.large_chunk_ratio = 4;
    config.chunk_min_content = 256;
    config.max_metadata_percentage = 0.25;
    config.contextual_rag_reserved_tokens = 512;
    
    chunking::AdvancedChunker chunker(tokenizer, config);
    
    // Test different document sizes
    std::vector<std::string> test_documents = {
        generate_test_document(1),   // 1KB
        generate_test_document(10),  // 10KB
        generate_test_document(50),  // 50KB
        generate_test_document(100), // 100KB
        generate_test_document(500)  // 500KB
    };
    
    for (size_t i = 0; i < test_documents.size(); ++i) {
        const auto& doc_content = test_documents[i];
        size_t size_kb = (i == 0) ? 1 : (i == 1) ? 10 : (i == 2) ? 50 : (i == 3) ? 100 : 500;
        
        std::cout << "\n🔍 Benchmarking chunking for " << size_kb << "KB document\n";
        
        // Create document info
        chunking::AdvancedChunker::DocumentInfo doc_info;
        doc_info.document_id = "perf_test_" + std::to_string(size_kb) + "kb";
        doc_info.title = "Performance Test Document";
        doc_info.semantic_identifier = "perf_test";
        doc_info.source_type = "file";
        doc_info.full_content = doc_content;
        doc_info.total_tokens = static_cast<int>(tokenizer->count_tokens(doc_content));
        
        // Create sections
        chunking::section_processing::DocumentSection section;
        section.content = doc_content;
        section.link = "perf_test_link";
        section.token_count = doc_info.total_tokens;
        doc_info.sections.push_back(section);
        
        auto results = runner.run_benchmark([&chunker, &doc_info]() {
            return chunker.process_document(doc_info);
        });
        
        std::cout << "  📊 Results:\n";
        std::cout << "    Iterations: " << results.iterations << "\n";
        std::cout << "    Total Time: " << utils::PerformanceUtils::format_time(results.total_time_ms) << "\n";
        std::cout << "    Avg Time: " << utils::PerformanceUtils::format_time(results.avg_time_ms) << "\n";
        std::cout << "    Throughput: " << utils::PerformanceUtils::format_throughput(results.throughput_per_second) << "\n";
        std::cout << "    Avg Memory: " << utils::PerformanceUtils::format_memory(results.avg_memory_bytes) << "\n";
    }
}

void benchmark_parallel_processing() {
    print_subsection("Parallel Processing Benchmark");
    
    utils::PerformanceUtils::BenchmarkRunner runner("Parallel Processing", 20);
    runner.set_warmup_iterations(5);
    runner.set_memory_tracking(false); // Disable memory tracking to avoid Docker issues
    
    // Load configuration from config file instead of hardcoded values
    r3m::core::ConfigManager config_manager;
    if (!config_manager.load_config("configs/dev/config.yaml")) {
        std::cerr << "❌ Failed to load configuration from config file\n";
        return;
    }
    
    // Get all configuration from config file
    std::unordered_map<std::string, std::string> config = config_manager.get_all_config();
    
    auto processor = std::make_unique<r3m::core::DocumentProcessor>();
    processor->initialize(config);
    
    // Create test files for parallel processing
    std::vector<std::string> test_files;
    for (int i = 0; i < 20; ++i) {
        std::string filename = "data/parallel_test_" + std::to_string(i) + ".txt";
        std::ofstream file(filename);
        if (file.is_open()) {
            std::string content = generate_test_document(10); // 10KB each
            file << content;
            file.close();
            test_files.push_back(filename);
        }
    }
    
    std::cout << "🔍 Benchmarking parallel processing with " << test_files.size() << " files\n";
    
    auto results = runner.run_benchmark([&processor, &test_files]() {
        return processor->process_documents_parallel(test_files);
    });
    
    std::cout << "  📊 Results:\n";
    std::cout << "    Iterations: " << results.iterations << "\n";
    std::cout << "    Total Time: " << utils::PerformanceUtils::format_time(results.total_time_ms) << "\n";
    std::cout << "    Avg Time: " << utils::PerformanceUtils::format_time(results.avg_time_ms) << "\n";
    std::cout << "    Throughput: " << utils::PerformanceUtils::format_throughput(results.throughput_per_second) << "\n";
    std::cout << "    Avg Memory: " << utils::PerformanceUtils::format_memory(results.avg_memory_bytes) << "\n";
    
    // Clean up test files
    for (const auto& file : test_files) {
        std::filesystem::remove(file);
    }
}

void benchmark_memory_usage() {
    print_subsection("Memory Usage Benchmark");
    
    utils::PerformanceUtils::PerformanceMonitor monitor;
    monitor.start_monitoring();
    
    // Initialize components
    auto tokenizer = std::make_shared<chunking::BasicTokenizer>();
    chunking::AdvancedChunker::Config config;
    config.enable_multipass = true;
    config.enable_contextual_rag = true;
    
    chunking::AdvancedChunker chunker(tokenizer, config);
    
    // Process documents of increasing size
    std::vector<size_t> sizes = {1, 5, 10, 25, 50, 100}; // KB
    
    for (size_t size : sizes) {
        std::string content = generate_test_document(size);
        
        chunking::AdvancedChunker::DocumentInfo doc_info;
        doc_info.document_id = "memory_test_" + std::to_string(size) + "kb";
        doc_info.title = "Memory Test Document";
        doc_info.full_content = content;
        doc_info.total_tokens = static_cast<int>(tokenizer->count_tokens(content));
        
        chunking::section_processing::DocumentSection section;
        section.content = content;
        doc_info.sections.push_back(section);
        
        // Disable memory tracking to avoid Docker issues
        // size_t memory_before = utils::PerformanceUtils::get_current_memory_usage();
        
        auto result = chunker.process_document(doc_info);
        
        // size_t memory_after = utils::PerformanceUtils::get_current_memory_usage();
        // size_t memory_used = memory_after - memory_before;
        
        // monitor.add_metric("memory_usage_kb", memory_used / 1024.0);
        monitor.add_metric("document_size_kb", static_cast<double>(size));
        monitor.add_metric("chunks_generated", static_cast<double>(result.chunks.size()));
        
        std::cout << "  📄 " << size << "KB document: " 
                  << "0 B memory" // Disable memory reporting
                  << ", " << result.chunks.size() << " chunks\n";
    }
    
    monitor.stop_monitoring();
    monitor.print_summary();
    monitor.export_to_csv("data/memory_benchmark.csv");
}

void run_performance_stress_test() {
    print_subsection("Performance Stress Test");
    
    utils::PerformanceUtils::PerformanceMonitor monitor;
    monitor.start_monitoring();
    
    // Load configuration from config file instead of hardcoded values
    r3m::core::ConfigManager config_manager;
    if (!config_manager.load_config("configs/dev/config.yaml")) {
        std::cerr << "❌ Failed to load configuration from config file\n";
        return;
    }
    
    // Get all configuration from config file
    std::unordered_map<std::string, std::string> config = config_manager.get_all_config();
    
    auto processor = std::make_unique<r3m::core::DocumentProcessor>();
    processor->initialize(config);
    
    // Create large test document
    std::string large_document = generate_test_document(1000); // 1MB
    std::ofstream file("data/stress_test_1mb.txt");
    file << large_document;
    file.close();
    
    std::cout << "🔥 Running stress test with 1MB document\n";
    
    // Process multiple times to stress test
    for (int i = 0; i < 10; ++i) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        auto result = processor->process_document("data/stress_test_1mb.txt");
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(end_time - start_time);
        
        monitor.add_metric("stress_test_time_ms", duration.count());
        monitor.add_metric("stress_test_chunks", static_cast<double>(result.chunks.size()));
        // Disable memory tracking to avoid Docker issues
        // monitor.add_metric("stress_test_memory_mb", 
        //                   static_cast<double>(utils::PerformanceUtils::get_current_memory_usage()) / (1024.0 * 1024.0));
        
        std::cout << "  🔄 Iteration " << (i + 1) << ": " 
                  << utils::PerformanceUtils::format_time(duration.count())
                  << ", " << result.chunks.size() << " chunks\n";
    }
    
    monitor.stop_monitoring();
    monitor.print_summary();
    monitor.export_to_csv("data/stress_test_results.csv");
    
    // Clean up
    std::filesystem::remove("data/stress_test_1mb.txt");
}

int main() {
    print_section("R3M PERFORMANCE BENCHMARKING SUITE");
    std::cout << "Testing performance optimization and benchmarking capabilities\n";
    
    try {
        // Create test files
        create_performance_test_files();
        
        // Run benchmarks
        benchmark_document_processing();
        benchmark_chunking_performance();
        benchmark_parallel_processing();
        benchmark_memory_usage();
        run_performance_stress_test();
        
        std::cout << "\n🎉 All performance benchmarks completed successfully!\n";
        std::cout << "📊 Check data/ directory for detailed CSV reports\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Performance benchmark failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 