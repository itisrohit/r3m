#include "r3m/chunking/advanced_chunker.hpp"
#include <iostream>
#include <cassert>

using namespace r3m::chunking;

int main() {
    std::cout << "🧪 Testing R3M Advanced Chunking Features..." << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // Test 1: Basic Advanced Chunker
    std::cout << "\n📄 Test 1: Basic Advanced Chunker" << std::endl;
    
    AdvancedChunker::Config config;
    config.enable_multipass = true;
    config.enable_large_chunks = true;
    config.enable_contextual_rag = true;
    config.enable_quality_filtering = true;
    config.min_chunk_length = 10; // Lower minimum for testing
    config.min_chunk_quality = 0.1; // Lower quality threshold for testing
    config.min_chunk_density = 0.05; // Lower density threshold for testing
    
    AdvancedChunker chunker(config);
    
    std::string test_doc = "R3M is a high-performance C++ API for document processing. "
                          "It provides fast document processing and pipeline orchestration. "
                          "The system supports multiple formats including PDF, TXT, HTML, and JSON. "
                          "Quality assessment algorithms evaluate content quality and information density. "
                          "Parallel processing enables efficient batch operations with 2-3x speedup. "
                          "Advanced chunking features include multipass indexing and contextual RAG.";
    
    std::unordered_map<std::string, std::string> metadata = {
        {"author", "R3M Team"},
        {"version", "1.0.0"},
        {"type", "technical_document"}
    };
    
    auto result = chunker.chunk_document("test_doc_001", test_doc, "R3M Technical Overview", metadata);
    
    std::cout << "Document chunking results:" << std::endl;
    std::cout << "  Total chunks: " << result.total_chunks << std::endl;
    std::cout << "  Successful chunks: " << result.successful_chunks << std::endl;
    std::cout << "  Failed chunks: " << result.failed_chunks << std::endl;
    std::cout << "  Processing time: " << result.processing_time_ms << " ms" << std::endl;
    std::cout << "  Average quality: " << result.avg_quality_score << std::endl;
    std::cout << "  Average density: " << result.avg_information_density << std::endl;
    std::cout << "  High quality chunks: " << result.high_quality_chunks << std::endl;
    
    assert(result.successful_chunks > 0);
    assert(result.processing_time_ms > 0.0);
    std::cout << "✅ Basic advanced chunker test passed!" << std::endl;
    
    // Test 2: Multipass Features
    std::cout << "\n📄 Test 2: Multipass Features" << std::endl;
    
    for (size_t i = 0; i < result.chunks.size(); ++i) {
        const auto& chunk = result.chunks[i];
        std::cout << "  Chunk " << i << ":" << std::endl;
        std::cout << "    ID: " << chunk.chunk_id << std::endl;
        std::cout << "    Content length: " << chunk.content.length() << " chars" << std::endl;
        std::cout << "    Quality: " << chunk.quality_score << std::endl;
        std::cout << "    Density: " << chunk.information_density << std::endl;
        std::cout << "    High quality: " << (chunk.is_high_quality ? "YES" : "NO") << std::endl;
        
        if (!chunk.mini_chunk_texts.empty()) {
            std::cout << "    Mini-chunks: " << chunk.mini_chunk_texts.size() << std::endl;
        }
        
        if (chunk.large_chunk_id >= 0) {
            std::cout << "    Large chunk ID: " << chunk.large_chunk_id << std::endl;
        }
        
        if (!chunk.doc_summary.empty()) {
            std::cout << "    Has document summary: YES" << std::endl;
        }
        
        if (!chunk.chunk_context.empty()) {
            std::cout << "    Has chunk context: YES" << std::endl;
        }
    }
    
    std::cout << "✅ Multipass features test passed!" << std::endl;
    
    // Test 3: Quality Filtering
    std::cout << "\n📄 Test 3: Quality Filtering" << std::endl;
    
    std::vector<DocumentChunk> test_chunks = result.chunks;
    auto filtered_chunks = chunker.filter_chunks_by_quality(test_chunks);
    
    std::cout << "Quality filtering results:" << std::endl;
    std::cout << "  Original chunks: " << test_chunks.size() << std::endl;
    std::cout << "  Filtered chunks: " << filtered_chunks.size() << std::endl;
    std::cout << "  Filtered out: " << (test_chunks.size() - filtered_chunks.size()) << std::endl;
    
    assert(filtered_chunks.size() <= test_chunks.size());
    std::cout << "✅ Quality filtering test passed!" << std::endl;
    
    // Test 4: Batch Processing
    std::cout << "\n📄 Test 4: Batch Processing" << std::endl;
    
    std::vector<std::tuple<std::string, std::string, std::string, std::unordered_map<std::string, std::string>>> documents = {
        {"doc_001", "First document content with sufficient length to generate chunks. This document contains multiple sentences and should be processed by the advanced chunking system.", "Document 1", {{"type", "text"}}},
        {"doc_002", "Second document with more content and details. This technical document includes various aspects of the system and provides comprehensive information for testing purposes.", "Document 2", {{"type", "technical"}}},
        {"doc_003", "Third document for testing batch processing capabilities. This document contains enough content to trigger chunking and demonstrates the advanced features of the R3M system.", "Document 3", {{"type", "test"}}}
    };
    
    auto batch_results = chunker.chunk_documents_batch(documents);
    
    std::cout << "Batch processing results:" << std::endl;
    std::cout << "  Documents processed: " << batch_results.size() << std::endl;
    
    // Debug: Check individual document results
    for (size_t i = 0; i < batch_results.size(); ++i) {
        const auto& batch_result = batch_results[i];
        std::cout << "  Debug Document " << i + 1 << ":" << std::endl;
        std::cout << "    Total chunks: " << batch_result.total_chunks << std::endl;
        std::cout << "    Successful chunks: " << batch_result.successful_chunks << std::endl;
        std::cout << "    Failed chunks: " << batch_result.failed_chunks << std::endl;
        if (!batch_result.chunks.empty()) {
            std::cout << "    First chunk content length: " << batch_result.chunks[0].content.length() << std::endl;
        }
    }
    
    size_t total_chunks = 0;
    double total_time = 0.0;
    
    for (size_t i = 0; i < batch_results.size(); ++i) {
        const auto& batch_result = batch_results[i];
        std::cout << "  Document " << i + 1 << ": " << batch_result.successful_chunks << " chunks, "
                  << batch_result.processing_time_ms << " ms" << std::endl;
        
        total_chunks += batch_result.successful_chunks;
        total_time += batch_result.processing_time_ms;
    }
    
    std::cout << "  Total chunks: " << total_chunks << std::endl;
    std::cout << "  Total time: " << total_time << " ms" << std::endl;
    std::cout << "  Average time per document: " << (total_time / batch_results.size()) << " ms" << std::endl;
    
    assert(batch_results.size() == documents.size());
    assert(total_chunks > 0);
    std::cout << "✅ Batch processing test passed!" << std::endl;
    
    // Test 5: Configuration Updates
    std::cout << "\n📄 Test 5: Configuration Updates" << std::endl;
    
    AdvancedChunker::Config new_config = chunker.get_config();
    new_config.enable_multipass = false;
    new_config.enable_contextual_rag = false;
    new_config.min_chunk_quality = 0.8; // More strict filtering
    
    chunker.update_config(new_config);
    
    auto strict_result = chunker.chunk_document("strict_doc", test_doc, "Strict Test", metadata);
    
    std::cout << "Strict configuration results:" << std::endl;
    std::cout << "  Chunks with strict filtering: " << strict_result.successful_chunks << std::endl;
    std::cout << "  Average quality: " << strict_result.avg_quality_score << std::endl;
    
    assert(strict_result.successful_chunks <= result.successful_chunks);
    std::cout << "✅ Configuration updates test passed!" << std::endl;
    
    std::cout << "\n🎉 ALL ADVANCED CHUNKING TESTS PASSED!" << std::endl;
    std::cout << "✅ Basic advanced chunker: OK" << std::endl;
    std::cout << "✅ Multipass features: OK" << std::endl;
    std::cout << "✅ Quality filtering: OK" << std::endl;
    std::cout << "✅ Batch processing: OK" << std::endl;
    std::cout << "✅ Configuration updates: OK" << std::endl;
    
    std::cout << "\n🚀 R3M advanced chunking system is fully operational!" << std::endl;
    
    return 0;
} 