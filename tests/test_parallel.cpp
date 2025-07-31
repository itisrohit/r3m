#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include "r3m/core/document_processor.hpp"

using namespace r3m::core;

int main() {
    std::cout << "R3M Parallel Processing Test\n";
    std::cout << "============================\n\n";

    // Set up processor
    DocumentProcessor processor;
    std::unordered_map<std::string, std::string> config;
    config["document_processing.max_file_size"] = "100MB";
    config["document_processing.max_text_length"] = "1000000";
    config["document_processing.text_processing.encoding_detection"] = "true";
    config["document_processing.text_processing.default_encoding"] = "utf-8";
    config["document_processing.text_processing.remove_html_tags"] = "true";
    config["document_processing.text_processing.normalize_whitespace"] = "true";
    config["document_processing.text_processing.extract_metadata"] = "true";
    config["document_processing.batch_size"] = "4";
    config["document_processing.max_workers"] = "2";
    processor.initialize(config);

    // Create test files
    std::vector<std::string> test_files;
    for (int i = 0; i < 8; ++i) {
        std::string filename = "parallel_test_" + std::to_string(i) + ".txt";
        std::string filepath = "../data/" + filename;
        
        std::ofstream test_file(filepath);
        test_file << "This is parallel test file " << i << " for R3M.\n";
        test_file << "Testing parallel processing capabilities.\n";
        test_file << "File number: " << i << "\n";
        
        // Create larger content to make processing take more time
        for (int j = 0; j < 1000; ++j) {
            test_file << "This is line " << j << " of additional content for file " << i << ".\n";
            test_file << "Adding more content to make processing take longer.\n";
            test_file << "Line " << j << " contains some sample text for testing.\n";
            test_file << "This is more content to increase file size and processing time.\n";
        }
        
        test_file << "Final line for file " << i << ".\n";
        test_file.close();
        
        test_files.push_back(filepath);
    }

    std::cout << "Created " << test_files.size() << " test files\n\n";

    // Test 1: Sequential processing
    std::cout << "1. Sequential Processing Test\n";
    std::cout << "-----------------------------\n";
    
    auto seq_start = std::chrono::high_resolution_clock::now();
    std::vector<DocumentResult> seq_results;
    for (const auto& file : test_files) {
        seq_results.push_back(processor.process_document(file));
    }
    auto seq_end = std::chrono::high_resolution_clock::now();
    
    auto seq_duration = std::chrono::duration_cast<std::chrono::milliseconds>(seq_end - seq_start);
    std::cout << "Sequential processing time: " << seq_duration.count() << " ms\n";
    std::cout << "Average time per file: " << (seq_duration.count() / test_files.size()) << " ms\n\n";

    // Test 2: Parallel processing
    std::cout << "2. Parallel Processing Test\n";
    std::cout << "---------------------------\n";
    
    auto par_start = std::chrono::high_resolution_clock::now();
    auto par_results = processor.process_documents_parallel(test_files);
    auto par_end = std::chrono::high_resolution_clock::now();
    
    auto par_duration = std::chrono::duration_cast<std::chrono::milliseconds>(par_end - par_start);
    std::cout << "Parallel processing time: " << par_duration.count() << " ms\n";
    std::cout << "Average time per file: " << (par_duration.count() / test_files.size()) << " ms\n\n";

    // Test 3: Batch processing
    std::cout << "3. Batch Processing Test\n";
    std::cout << "------------------------\n";
    
    auto batch_start = std::chrono::high_resolution_clock::now();
    auto batch_results = processor.process_documents_batch(test_files);
    auto batch_end = std::chrono::high_resolution_clock::now();
    
    auto batch_duration = std::chrono::duration_cast<std::chrono::milliseconds>(batch_end - batch_start);
    std::cout << "Batch processing time: " << batch_duration.count() << " ms\n";
    std::cout << "Average time per file: " << (batch_duration.count() / test_files.size()) << " ms\n\n";

    // Performance comparison
    std::cout << "4. Performance Comparison\n";
    std::cout << "-------------------------\n";
    std::cout << "Sequential: " << seq_duration.count() << " ms\n";
    std::cout << "Parallel:   " << par_duration.count() << " ms\n";
    std::cout << "Batch:      " << batch_duration.count() << " ms\n\n";

    if (par_duration.count() > 0) {
        double speedup = (double)seq_duration.count() / par_duration.count();
        std::cout << "Parallel speedup: " << speedup << "x\n";
    }

    if (batch_duration.count() > 0) {
        double speedup = (double)seq_duration.count() / batch_duration.count();
        std::cout << "Batch speedup: " << speedup << "x\n";
    }

    // Verify results
    std::cout << "\n5. Result Verification\n";
    std::cout << "----------------------\n";
    std::cout << "Sequential results: " << seq_results.size() << " files\n";
    std::cout << "Parallel results:   " << par_results.size() << " files\n";
    std::cout << "Batch results:      " << batch_results.size() << " files\n";

    int seq_success = 0, par_success = 0, batch_success = 0;
    for (const auto& result : seq_results) if (result.processing_success) seq_success++;
    for (const auto& result : par_results) if (result.processing_success) par_success++;
    for (const auto& result : batch_results) if (result.processing_success) batch_success++;

    std::cout << "Sequential success: " << seq_success << "/" << seq_results.size() << "\n";
    std::cout << "Parallel success:   " << par_success << "/" << par_results.size() << "\n";
    std::cout << "Batch success:      " << batch_success << "/" << batch_results.size() << "\n";

    return 0;
} 