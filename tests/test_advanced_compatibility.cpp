#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include "r3m/chunking/advanced_chunker.hpp"
#include "r3m/chunking/tokenizer.hpp"
#include "r3m/chunking/chunk_models.hpp"

using namespace r3m::chunking;

void test_advanced_section_combination_logic() {
    std::cout << "Testing advanced section combination logic..." << std::endl;
    
    // Create a simple tokenizer
    auto tokenizer = std::make_shared<BasicTokenizer>();
    
    // Create advanced chunker with advanced configuration
    AdvancedChunker::Config config;
    config.chunk_token_limit = 100;  // Small limit for testing
    config.enable_multipass = true;
    config.enable_contextual_rag = true;
    config.include_metadata = true;
    
    AdvancedChunker chunker(tokenizer, config);
    
    // Create document with multiple sections
    AdvancedChunker::DocumentInfo document;
    document.document_id = "test_doc";
    document.title = "Test Document";
    document.source_type = "file";
    document.semantic_identifier = "test_doc_001";
    
    // Add sections that should be combined
    document.sections.push_back({"This is the first section with some content.", "link1"});
    document.sections.push_back({"This is the second section that should combine.", "link2"});
    document.sections.push_back({"This is the third section.", "link3"});
    
    // Process document
    auto result = chunker.process_document(document);
    
    // Verify that sections were combined (fewer chunks than sections)
    assert(result.chunks.size() < document.sections.size());
    std::cout << "✅ Section combination logic test passed!" << std::endl;
}

void test_advanced_strict_token_limit() {
    std::cout << "Testing advanced strict token limit enforcement..." << std::endl;
    
    auto tokenizer = std::make_shared<BasicTokenizer>();
    
    AdvancedChunker::Config config;
    config.chunk_token_limit = 50;  // Very small limit
    config.enable_multipass = false;
    config.enable_contextual_rag = false;
    
    AdvancedChunker chunker(tokenizer, config);
    
    // Create document with oversized content
    AdvancedChunker::DocumentInfo document;
    document.document_id = "oversized_doc";
    document.title = "Oversized Document";
    document.source_type = "file";
    document.semantic_identifier = "oversized_doc_001";
    
    // Add a very long section that exceeds token limit
    std::string long_content = "This is a very long section that contains many words and should definitely exceed the token limit that we have set for this test. It should be split into multiple chunks to ensure that no single chunk exceeds the maximum token limit.";
    document.sections.push_back({long_content, "link1"});
    
    auto result = chunker.process_document(document);
    
    // Verify that oversized content was split
    assert(result.chunks.size() > 1);
    
    // Verify that no chunk exceeds the token limit
    for (const auto& chunk : result.chunks) {
        int chunk_tokens = static_cast<int>(tokenizer->count_tokens(chunk.content));
        assert(chunk_tokens <= config.chunk_token_limit);
        (void)chunk_tokens; // Suppress unused variable warning
    }
    
    std::cout << "✅ Strict token limit enforcement test passed!" << std::endl;
}

void test_advanced_link_offset_tracking() {
    std::cout << "Testing advanced link offset tracking..." << std::endl;
    
    auto tokenizer = std::make_shared<BasicTokenizer>();
    
    AdvancedChunker::Config config;
    config.chunk_token_limit = 100;
    config.enable_multipass = false;
    config.enable_contextual_rag = false;
    
    AdvancedChunker chunker(tokenizer, config);
    
    // Create document with multiple sections
    AdvancedChunker::DocumentInfo document;
    document.document_id = "link_test_doc";
    document.title = "Link Test Document";
    document.source_type = "file";
    document.semantic_identifier = "link_test_001";
    
    document.sections.push_back({"First section content.", "link1"});
    document.sections.push_back({"Second section content.", "link2"});
    document.sections.push_back({"Third section content.", "link3"});
    
    auto result = chunker.process_document(document);
    
    // Verify that chunks have link offsets
    bool found_links = false;
    for (const auto& chunk : result.chunks) {
        if (!chunk.source_links.empty()) {
            found_links = true;
            break;
        }
    }
    
    assert(found_links);
    (void)found_links; // Suppress unused variable warning
    std::cout << "✅ Link offset tracking test passed!" << std::endl;
}

void test_advanced_contextual_rag() {
    std::cout << "Testing advanced contextual RAG..." << std::endl;
    
    auto tokenizer = std::make_shared<BasicTokenizer>();
    
    AdvancedChunker::Config config;
    config.chunk_token_limit = 200;
    config.enable_multipass = true;
    config.enable_contextual_rag = true;
    config.contextual_rag_reserved_tokens = 50;
    
    AdvancedChunker chunker(tokenizer, config);
    
    // Create document that won't fit in single chunk
    AdvancedChunker::DocumentInfo document;
    document.document_id = "rag_test_doc";
    document.title = "RAG Test Document";
    document.source_type = "file";
    document.semantic_identifier = "rag_test_001";
    
    // Add content that exceeds single chunk limit
    std::string long_content = "This is a very long document that contains many sections and paragraphs. It should definitely exceed the single chunk limit and trigger contextual RAG processing. The document should be split into multiple chunks with contextual information added to each chunk.";
    document.sections.push_back({long_content, "link1"});
    
    auto result = chunker.process_document(document);
    
    // Verify that contextual RAG was applied
    bool found_rag_content = false;
    for (const auto& chunk : result.chunks) {
        if (chunk.contextual_rag_reserved_tokens > 0) {
            found_rag_content = true;
            break;
        }
    }
    
    assert(found_rag_content);
    (void)found_rag_content; // Suppress unused variable warning
    std::cout << "✅ Advanced contextual RAG test passed!" << std::endl;
}

void test_advanced_source_specific_handling() {
    std::cout << "Testing advanced source-specific handling..." << std::endl;
    
    auto tokenizer = std::make_shared<BasicTokenizer>();
    
    AdvancedChunker::Config config;
    config.chunk_token_limit = 100;
    config.enable_multipass = false;
    config.enable_contextual_rag = false;
    
    AdvancedChunker chunker(tokenizer, config);
    
    // Test Gmail source type
    AdvancedChunker::DocumentInfo gmail_doc;
    gmail_doc.document_id = "gmail_test";
    gmail_doc.title = "Gmail Test";
    gmail_doc.source_type = "gmail";
    gmail_doc.semantic_identifier = "gmail_test_001";
    gmail_doc.sections.push_back({"Gmail content here.", "gmail_link"});
    
    auto gmail_result = chunker.process_document(gmail_doc);
    
    // Verify Gmail document was processed
    assert(!gmail_result.chunks.empty());
    assert(gmail_result.chunks[0].source_type == "gmail");
    
    std::cout << "✅ Source-specific handling test passed!" << std::endl;
}

void test_advanced_constants_and_configuration() {
    std::cout << "Testing advanced constants and configuration..." << std::endl;
    
    // Test that all advanced constants are available
    assert(MAX_METADATA_PERCENTAGE == 0.25);
    assert(CHUNK_MIN_CONTENT == 256);
    assert(CHUNK_OVERLAP == 0);
    assert(STRICT_CHUNK_TOKEN_LIMIT == true);
    assert(AVERAGE_SUMMARY_EMBEDDINGS == false);
    assert(USE_CHUNK_SUMMARY == true);
    assert(USE_DOCUMENT_SUMMARY == true);
    
    std::cout << "✅ Constants and configuration test passed!" << std::endl;
}

int main() {
    std::cout << "🧪 Running Advanced Compatibility Tests" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    try {
        test_advanced_section_combination_logic();
        test_advanced_strict_token_limit();
        test_advanced_link_offset_tracking();
        test_advanced_contextual_rag();
        test_advanced_source_specific_handling();
        test_advanced_constants_and_configuration();
        
        std::cout << "\n🎉 All advanced compatibility tests passed successfully!" << std::endl;
        std::cout << "✅ R3M now has advanced chunking capabilities!" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
} 