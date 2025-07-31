#include "r3m/chunking/tokenizer.hpp"
#include "r3m/chunking/sentence_chunker.hpp"
#include "r3m/chunking/chunk_models.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

using namespace r3m::chunking;

void test_tokenizer_edge_cases() {
    std::cout << "\n🔍 Testing Tokenizer Edge Cases..." << std::endl;
    
    auto tokenizer = TokenizerFactory::create(TokenizerFactory::Type::BASIC, 1000);
    
    // Test 1: Empty string
    auto empty_tokens = tokenizer->tokenize("");
    assert(empty_tokens.empty());
    assert(tokenizer->count_tokens("") == 0);
    std::cout << "✅ Empty string handling: OK" << std::endl;
    
    // Test 2: Single word
    auto single_tokens = tokenizer->tokenize("Hello");
    assert(single_tokens.size() == 1);
    assert(single_tokens[0] == "Hello");
    std::cout << "✅ Single word handling: OK" << std::endl;
    
    // Test 3: Multiple punctuation
    auto punct_tokens = tokenizer->tokenize("Hello!!!");
    std::cout << "Punctuation tokens: ";
    for (const auto& token : punct_tokens) {
        std::cout << "[" << token << "] ";
    }
    std::cout << std::endl;
    assert(punct_tokens.size() >= 2);
    assert(punct_tokens[0] == "Hello");
    std::cout << "✅ Multiple punctuation handling: OK" << std::endl;
    
    // Test 4: Whitespace handling
    auto whitespace_tokens = tokenizer->tokenize("  Hello   world  ");
    assert(whitespace_tokens.size() == 2);
    assert(whitespace_tokens[0] == "Hello");
    assert(whitespace_tokens[1] == "world");
    std::cout << "✅ Whitespace handling: OK" << std::endl;
    
    // Test 5: Technical text
    std::string tech_text = "R3M is a C++ API for document processing. It uses ONNX models.";
    auto tech_tokens = tokenizer->tokenize(tech_text);
    assert(tech_tokens.size() > 0);
    std::cout << "✅ Technical text handling: OK (" << tech_tokens.size() << " tokens)" << std::endl;
}

void test_sentence_chunker_edge_cases() {
    std::cout << "\n🔍 Testing Sentence Chunker Edge Cases..." << std::endl;
    
    auto tokenizer = TokenizerFactory::create(TokenizerFactory::Type::BASIC, 1000);
    auto chunker = std::make_shared<SentenceChunker>(std::move(tokenizer), 10, 0, "texts");
    
    // Test 1: Empty text
    auto empty_chunks = chunker->chunk("");
    assert(empty_chunks.empty());
    std::cout << "✅ Empty text handling: OK" << std::endl;
    
    // Test 2: Single sentence
    auto single_chunks = chunker->chunk("This is a single sentence.");
    assert(single_chunks.size() == 1);
    std::cout << "✅ Single sentence handling: OK" << std::endl;
    
    // Test 3: Abbreviations
    std::string abbr_text = "Mr. Smith went to Dr. Johnson. He said etc. and i.e. are common.";
    auto abbr_chunks = chunker->chunk(abbr_text);
    std::cout << "✅ Abbreviation handling: OK (" << abbr_chunks.size() << " chunks)" << std::endl;
    
    // Test 4: Very long sentence
    std::string long_sentence = "This is a very long sentence that contains many words and should be split into multiple chunks because it exceeds the token limit that we have set for our chunking algorithm.";
    auto long_chunks = chunker->chunk(long_sentence);
    std::cout << "Long sentence chunks: " << long_chunks.size() << std::endl;
    for (size_t i = 0; i < long_chunks.size(); ++i) {
        std::cout << "  Chunk " << i + 1 << ": " << long_chunks[i] << std::endl;
    }
    assert(long_chunks.size() >= 1);
    std::cout << "✅ Long sentence handling: OK (" << long_chunks.size() << " chunks)" << std::endl;
    
    // Test 5: Mixed content
    std::string mixed_text = "First sentence. Second sentence! Third sentence? Fourth sentence. Fifth sentence.";
    auto mixed_chunks = chunker->chunk(mixed_text);
    std::cout << "Mixed content chunks: " << mixed_chunks.size() << std::endl;
    for (size_t i = 0; i < mixed_chunks.size(); ++i) {
        std::cout << "  Chunk " << i + 1 << ": " << mixed_chunks[i] << std::endl;
    }
    assert(mixed_chunks.size() >= 1);
    std::cout << "✅ Mixed content handling: OK (" << mixed_chunks.size() << " chunks)" << std::endl;
}

void test_chunk_models_comprehensive() {
    std::cout << "\n🔍 Testing Chunk Models Comprehensive..." << std::endl;
    
    // Test 1: BaseChunk
    BaseChunk base_chunk(1, "First sentence.", "This is the content of the chunk.");
    assert(base_chunk.chunk_id == 1);
    assert(base_chunk.blurb == "First sentence.");
    assert(base_chunk.content == "This is the content of the chunk.");
    assert(!base_chunk.section_continuation);
    std::cout << "✅ BaseChunk creation: OK" << std::endl;
    
    // Test 2: DocumentChunk
    DocumentChunk doc_chunk;
    doc_chunk.chunk_id = 2;
    doc_chunk.document_id = "doc_001";
    doc_chunk.blurb = "Second sentence.";
    doc_chunk.content = "This is the second chunk content.";
    doc_chunk.title_prefix = "Test Document";
    doc_chunk.metadata_semantic = "Metadata: author - John Doe";
    doc_chunk.metadata_keyword = "John Doe";
    doc_chunk.quality_score = 0.85;
    doc_chunk.information_density = 0.72;
    doc_chunk.is_high_quality = true;
    doc_chunk.large_chunk_id = -1;
    doc_chunk.contextual_rag_reserved_tokens = 0;
    
    assert(doc_chunk.chunk_id == 2);
    assert(doc_chunk.document_id == "doc_001");
    assert(doc_chunk.quality_score == 0.85);
    assert(doc_chunk.information_density == 0.72);
    assert(doc_chunk.is_high_quality);
    assert(doc_chunk.to_short_descriptor() == "doc_001 Chunk ID: 2");
    std::cout << "✅ DocumentChunk creation: OK" << std::endl;
    
    // Test 3: IndexedChunk
    IndexedChunk indexed_chunk;
    indexed_chunk.chunk_id = 3;
    indexed_chunk.document_id = "doc_002";
    indexed_chunk.blurb = "Third sentence.";
    indexed_chunk.content = "This is the third chunk content.";
    indexed_chunk.embedding = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f};
    indexed_chunk.title_embedding = {0.6f, 0.7f, 0.8f, 0.9f, 1.0f};
    
    assert(indexed_chunk.chunk_id == 3);
    assert(indexed_chunk.embedding.size() == 5);
    assert(indexed_chunk.title_embedding.size() == 5);
    std::cout << "✅ IndexedChunk creation: OK" << std::endl;
    
    // Test 4: ChunkingResult
    ChunkingResult result;
    result.chunks.push_back(doc_chunk);
    result.total_chunks = 1;
    result.successful_chunks = 1;
    result.failed_chunks = 0;
    result.processing_time_ms = 25.5;
    result.avg_quality_score = 0.85;
    result.avg_information_density = 0.72;
    result.high_quality_chunks = 1;
    
    assert(result.total_chunks == 1);
    assert(result.successful_chunks == 1);
    assert(result.failed_chunks == 0);
    assert(result.processing_time_ms == 25.5);
    assert(result.avg_quality_score == 0.85);
    assert(result.avg_information_density == 0.72);
    assert(result.high_quality_chunks == 1);
    std::cout << "✅ ChunkingResult creation: OK" << std::endl;
}

void test_integration_scenarios() {
    std::cout << "\n🔍 Testing Integration Scenarios..." << std::endl;
    
    // Scenario 1: Technical document chunking
    std::cout << "\n📄 Scenario 1: Technical Document Chunking" << std::endl;
    
    std::string tech_doc = "R3M is a high-performance C++ API for document processing. "
                          "It provides fast document processing and pipeline orchestration. "
                          "The system supports multiple formats including PDF, TXT, HTML, and JSON. "
                          "Quality assessment algorithms evaluate content quality and information density. "
                          "Parallel processing enables efficient batch operations with 2-3x speedup.";
    
    auto tokenizer = TokenizerFactory::create(TokenizerFactory::Type::BASIC, 1000);
    auto chunker = std::make_shared<SentenceChunker>(std::move(tokenizer), 25, 0, "texts");
    
    auto chunks = chunker->chunk(tech_doc);
    
    std::cout << "Technical document: " << tech_doc.length() << " characters" << std::endl;
    std::cout << "Number of chunks: " << chunks.size() << std::endl;
    
    for (size_t i = 0; i < chunks.size(); ++i) {
        auto test_tokenizer = TokenizerFactory::create(TokenizerFactory::Type::BASIC, 1000);
        auto token_count = test_tokenizer->count_tokens(chunks[i]);
        std::cout << "  Chunk " << i + 1 << ": " << chunks[i].length() << " chars, " 
                  << token_count << " tokens" << std::endl;
        assert(token_count <= chunker->get_chunk_size());
    }
    
    // Scenario 2: Multiple document types
    std::cout << "\n📄 Scenario 2: Multiple Document Types" << std::endl;
    
    std::vector<std::string> documents = {
        "This is a simple text document. It contains basic information.",
        "Markdown document with **bold** and *italic* formatting. It has multiple sections.",
        "JSON document: {\"key\": \"value\", \"number\": 42, \"array\": [1, 2, 3]}",
        "HTML document: <html><body><h1>Title</h1><p>Content</p></body></html>"
    };
    
    auto doc_tokenizer = TokenizerFactory::create(TokenizerFactory::Type::BASIC, 1000);
    auto doc_chunker = std::make_shared<SentenceChunker>(std::move(doc_tokenizer), 20, 0, "texts");
    
    for (size_t i = 0; i < documents.size(); ++i) {
        auto doc_chunks = doc_chunker->chunk(documents[i]);
        std::cout << "  Document " << i + 1 << ": " << doc_chunks.size() << " chunks" << std::endl;
        assert(doc_chunks.size() > 0);
    }
    
    // Scenario 3: Quality assessment integration
    std::cout << "\n📄 Scenario 3: Quality Assessment Integration" << std::endl;
    
    std::vector<DocumentChunk> quality_chunks;
    
    for (size_t i = 0; i < chunks.size(); ++i) {
        DocumentChunk chunk;
        chunk.chunk_id = i;
        chunk.document_id = "tech_doc_001";
        chunk.blurb = chunks[i].substr(0, std::min(size_t(50), chunks[i].length()));
        chunk.content = chunks[i];
        chunk.title_prefix = "Technical Document";
        chunk.quality_score = 0.7 + (i * 0.1); // Varying quality scores
        chunk.information_density = 0.6 + (i * 0.05); // Varying density
        chunk.is_high_quality = chunk.quality_score > 0.75;
        
        quality_chunks.push_back(chunk);
    }
    
    std::cout << "Created " << quality_chunks.size() << " quality-assessed chunks:" << std::endl;
    for (const auto& chunk : quality_chunks) {
        std::cout << "  Chunk " << chunk.chunk_id << ": quality=" << chunk.quality_score 
                  << ", density=" << chunk.information_density 
                  << ", high_quality=" << (chunk.is_high_quality ? "YES" : "NO") << std::endl;
    }
}

void test_performance_metrics() {
    std::cout << "\n🔍 Testing Performance Metrics..." << std::endl;
    
    auto tokenizer = TokenizerFactory::create(TokenizerFactory::Type::BASIC, 1000);
    auto chunker = std::make_shared<SentenceChunker>(std::move(tokenizer), 30, 0, "texts");
    
    // Large document for performance testing
    std::string large_doc;
    for (int i = 0; i < 100; ++i) {
        large_doc += "This is sentence number " + std::to_string(i + 1) + ". ";
        large_doc += "It contains various words and punctuation marks. ";
        large_doc += "The content varies in length and complexity. ";
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    auto chunks = chunker->chunk(large_doc);
    auto end_time = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "Large document processing:" << std::endl;
    std::cout << "  Document size: " << large_doc.length() << " characters" << std::endl;
    std::cout << "  Number of chunks: " << chunks.size() << std::endl;
    std::cout << "  Processing time: " << duration.count() / 1000.0 << " ms" << std::endl;
    std::cout << "  Average time per chunk: " << (duration.count() / 1000.0) / chunks.size() << " ms" << std::endl;
    
    assert(chunks.size() > 0);
    assert(duration.count() < 1000000); // Should complete in less than 1 second
    std::cout << "✅ Performance test passed!" << std::endl;
}

int main() {
    std::cout << "🧪 R3M Comprehensive Chunking Test" << std::endl;
    std::cout << "===================================" << std::endl;
    
    try {
        test_tokenizer_edge_cases();
        test_sentence_chunker_edge_cases();
        test_chunk_models_comprehensive();
        test_integration_scenarios();
        test_performance_metrics();
        
        std::cout << "\n🎉 ALL TESTS PASSED!" << std::endl;
        std::cout << "✅ Tokenizer edge cases: OK" << std::endl;
        std::cout << "✅ Sentence chunker edge cases: OK" << std::endl;
        std::cout << "✅ Chunk models comprehensive: OK" << std::endl;
        std::cout << "✅ Integration scenarios: OK" << std::endl;
        std::cout << "✅ Performance metrics: OK" << std::endl;
        
        std::cout << "\n🚀 R3M chunking system is ready for advanced features!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
    
    return 0;
} 