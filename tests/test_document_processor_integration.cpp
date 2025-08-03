#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <memory>
#include <unordered_map>

#include "r3m/core/document_processor.hpp"
#include "r3m/chunking/advanced_chunker.hpp"
#include "r3m/chunking/tokenizer.hpp"

void test_document_processor_chunking_integration() {
    std::cout << "Testing DocumentProcessor + AdvancedChunker integration..." << std::endl;
    
    // Create test document
    std::string test_file = "test_document_processor_integration.txt";
    std::string test_content = R"(
# Technical Documentation

This is a comprehensive technical document about R3M document processing system.

## Overview
R3M is a high-performance document processing engine written in C++. It provides advanced chunking capabilities with sophisticated token management and quality assessment.

## Features
- Advanced chunking with token-aware processing
- Multipass indexing for mini-chunks and large chunks
- Contextual RAG with document and chunk summaries
- Quality filtering and information density calculation
- Metadata processing with semantic and keyword extraction
- Section combination and oversized chunk handling

## Architecture
The system consists of several key components:
1. DocumentProcessor - Main orchestrator
2. AdvancedChunker - Sophisticated chunking engine
3. Tokenizer - Multiple tokenization strategies
4. QualityAssessor - Content quality evaluation
5. FormatProcessor - Multi-format document handling

## Performance
- Parallel processing with configurable thread pools
- Batch processing for large document collections
- Memory-efficient data structures
- Optimized token counting and management

## Configuration
All parameters are externalized to configuration files, allowing for flexible deployment scenarios.
    )";
    
    // Write test file
    std::ofstream file(test_file);
    file << test_content;
    file.close();
    
    // Initialize DocumentProcessor with chunking enabled
    auto processor = std::make_unique<r3m::core::DocumentProcessor>();
    
    std::unordered_map<std::string, std::string> config;
    config["document_processing.enable_chunking"] = "true";
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
    
    bool initialized = processor->initialize(config);
    assert(initialized);
    (void)initialized; // Suppress unused variable warning
    std::cout << "✅ DocumentProcessor initialized successfully" << std::endl;
    
    // Test single document processing with chunking
    auto result = processor->process_document(test_file);
    
    // Verify basic processing
    assert(result.processing_success);
    assert(!result.text_content.empty());
    assert(result.file_name == "test_document_processor_integration.txt");
    std::cout << "✅ Basic document processing successful" << std::endl;
    
    // Verify chunking results
    assert(result.total_chunks > 0);
    assert(result.successful_chunks > 0);
    assert(result.avg_chunk_quality > 0.0);
    assert(result.avg_chunk_density > 0.0);
    std::cout << "✅ Chunking results present" << std::endl;
    
    // Verify individual chunks
    for (const auto& chunk : result.chunks) {
        (void)chunk; // Suppress unused variable warning
        assert(chunk.chunk_id >= 0);
        assert(!chunk.content.empty());
        assert(!chunk.blurb.empty());
        assert(chunk.quality_score >= 0.0 && chunk.quality_score <= 1.0);
        assert(chunk.information_density >= 0.0 && chunk.information_density <= 1.0);
        assert(chunk.title_tokens >= 0);
        assert(chunk.metadata_tokens >= 0);
        assert(chunk.content_token_limit > 0);
        assert(!chunk.document_id.empty());
        assert(!chunk.source_type.empty());
        assert(!chunk.semantic_identifier.empty());
    }
    std::cout << "✅ Individual chunks validated" << std::endl;
    
    // Test chunking-specific method
    auto chunking_result = processor->process_document_with_chunking(test_file);
    
    assert(chunking_result.total_chunks > 0);
    assert(chunking_result.successful_chunks > 0);
    assert(chunking_result.failed_chunks == 0);
    assert(chunking_result.processing_time_ms > 0.0);
    assert(chunking_result.avg_quality_score > 0.0);
    assert(chunking_result.avg_information_density > 0.0);
    assert(chunking_result.high_quality_chunks > 0);
    assert(chunking_result.total_title_tokens > 0);
    assert(chunking_result.total_metadata_tokens > 0);
    assert(chunking_result.total_content_tokens > 0);
    std::cout << "✅ Chunking-specific processing successful" << std::endl;
    
    // Test batch processing with chunking
    std::vector<std::string> test_files = {test_file};
    auto batch_results = processor->process_documents_with_chunking(test_files);
    
    assert(batch_results.size() == 1);
    assert(batch_results[0].total_chunks > 0);
    assert(batch_results[0].successful_chunks > 0);
    std::cout << "✅ Batch processing with chunking successful" << std::endl;
    
    // Test parallel processing with chunking
    auto parallel_results = processor->process_documents_parallel(test_files);
    
    assert(parallel_results.size() == 1);
    assert(parallel_results[0].processing_success);
    assert(parallel_results[0].total_chunks > 0);
    assert(parallel_results[0].successful_chunks > 0);
    std::cout << "✅ Parallel processing with chunking successful" << std::endl;
    
    // Verify chunking statistics
    auto stats = processor->get_processing_stats();
    assert(stats.total_files_processed > 0);
    assert(stats.successful_processing > 0);
    (void)stats; // Suppress unused variable warning
    std::cout << "✅ Processing statistics updated" << std::endl;
    
    // Clean up
    std::filesystem::remove(test_file);
    
    std::cout << "✅ DocumentProcessor + AdvancedChunker integration test passed!" << std::endl;
}

void test_chunking_configuration() {
    std::cout << "Testing chunking configuration options..." << std::endl;
    
    // Create test document
    std::string test_file = "test_chunking_config.txt";
    std::string test_content = "This is a test document for chunking configuration testing. It contains multiple sentences and should be processed according to the configured chunking parameters.";
    
    std::ofstream file(test_file);
    file << test_content;
    file.close();
    
    // Test different configurations
    std::vector<std::unordered_map<std::string, std::string>> configs = {
        // Basic chunking
        {
            {"document_processing.enable_chunking", "true"},
            {"chunking.chunk_token_limit", "1024"},
            {"chunking.include_metadata", "false"}
        },
        // Advanced chunking with multipass
        {
            {"document_processing.enable_chunking", "true"},
            {"chunking.enable_multipass", "true"},
            {"chunking.enable_large_chunks", "true"},
            {"chunking.chunk_token_limit", "2048"},
            {"chunking.mini_chunk_size", "150"},
            {"chunking.large_chunk_ratio", "4"}
        },
        // Contextual RAG enabled
        {
            {"document_processing.enable_chunking", "true"},
            {"chunking.enable_contextual_rag", "true"},
            {"chunking.contextual_rag_reserved_tokens", "512"},
            {"chunking.chunk_token_limit", "2048"}
        }
    };
    
    for (size_t i = 0; i < configs.size(); ++i) {
        auto processor = std::make_unique<r3m::core::DocumentProcessor>();
        bool initialized = processor->initialize(configs[i]);
        assert(initialized);
        (void)initialized; // Suppress unused variable warning
        
        auto result = processor->process_document(test_file);
        assert(result.processing_success);
        assert(result.total_chunks > 0);
        
        std::cout << "  ✅ Configuration " << (i + 1) << " successful" << std::endl;
    }
    
    // Clean up
    std::filesystem::remove(test_file);
    
    std::cout << "✅ Chunking configuration test passed!" << std::endl;
}

void test_chunking_disabled() {
    std::cout << "Testing chunking disabled mode..." << std::endl;
    
    // Create test document
    std::string test_file = "test_chunking_disabled.txt";
    std::string test_content = "This is a test document for disabled chunking testing.";
    
    std::ofstream file(test_file);
    file << test_content;
    file.close();
    
    // Initialize DocumentProcessor with chunking disabled
    auto processor = std::make_unique<r3m::core::DocumentProcessor>();
    
    std::unordered_map<std::string, std::string> config;
    config["document_processing.enable_chunking"] = "false";
    
    bool initialized = processor->initialize(config);
    assert(initialized);
    (void)initialized; // Suppress unused variable warning
    
    // Test processing - should work but without chunking
    auto result = processor->process_document(test_file);
    assert(result.processing_success);
    assert(result.total_chunks == 0);
    assert(result.successful_chunks == 0);
    
    // Test chunking-specific method - should return empty result
    auto chunking_result = processor->process_document_with_chunking(test_file);
    assert(chunking_result.failed_chunks == 1);
    assert(chunking_result.successful_chunks == 0);
    
    // Clean up
    std::filesystem::remove(test_file);
    
    std::cout << "✅ Chunking disabled test passed!" << std::endl;
}

int main() {
    std::cout << "🚀 R3M DocumentProcessor + AdvancedChunker Integration Tests" << std::endl;
    std::cout << "Testing the integration between document processing and chunking systems" << std::endl;
    
    try {
        test_document_processor_chunking_integration();
        test_chunking_configuration();
        test_chunking_disabled();
        
        std::cout << "\n🎉 All integration tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
} 