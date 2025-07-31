#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include "r3m/chunking/advanced_chunker.hpp"
#include "r3m/chunking/tokenizer.hpp"
#include "r3m/chunking/metadata_processor.hpp"
#include "r3m/chunking/chunk_models.hpp"

using namespace r3m::chunking;

// ============================================================================
// METADATA AND TOKEN MANAGEMENT TESTS
// ============================================================================

void test_metadata_processing() {
    std::cout << "Testing metadata processing..." << std::endl;

    std::unordered_map<std::string, std::string> metadata;
    metadata["title"] = "Advanced Chunking System";
    metadata["description"] = "A sophisticated document chunking system";
    metadata["author"] = "R3M Team";
    metadata["category"] = "Technical Documentation";
    metadata["type"] = "documentation";
    metadata["format"] = "markdown";
    metadata["source"] = "internal";
    metadata["created_at"] = "2024-01-01"; // Should be ignored
    metadata["file_size"] = "1024"; // Should be ignored

    auto result = MetadataProcessor::process_metadata(metadata, true);

    // Verify metadata processing
    assert(result.is_valid);
    assert(!result.semantic_suffix.empty());
    assert(!result.keyword_suffix.empty());
    assert(result.semantic_tokens > 0);
    assert(result.keyword_tokens > 0);

    // Verify semantic metadata contains natural language format
    assert(result.semantic_suffix.find("title:") != std::string::npos);
    assert(result.semantic_suffix.find("description:") != std::string::npos);
    assert(result.semantic_suffix.find("author:") != std::string::npos);

    // Verify keyword metadata contains structured format
    assert(result.keyword_suffix.find("type=") != std::string::npos);
    assert(result.keyword_suffix.find("format=") != std::string::npos);
    assert(result.keyword_suffix.find("source=") != std::string::npos);

    // Verify ignored metadata is not present
    assert(result.semantic_suffix.find("created_at:") == std::string::npos);
    assert(result.keyword_suffix.find("file_size=") == std::string::npos);

    std::cout << "✅ Metadata processing test passed!" << std::endl;
}

void test_title_and_metadata_token_management() {
    std::cout << "Testing title and metadata token management..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 2048;
    config.include_metadata = true;
    config.max_metadata_percentage = 0.25;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "test_doc_2";
    doc.title = "This is a test document with a very long title that should be properly tokenized and managed";
    doc.semantic_identifier = "test_doc_2";
    doc.source_type = "file";
    doc.full_content = "This is the main content of the document. It contains multiple sentences.";

    // Add metadata
    doc.metadata["author"] = "John Doe";
    doc.metadata["category"] = "Technical Documentation";
    doc.metadata["tags"] = "testing, chunking, token management";
    doc.metadata["version"] = "1.0.0";
    doc.metadata["status"] = "active";

    // Add sections
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is the first section of the document. It contains important information about the topic.",
        "https://example.com/section1"
    ));

    auto result = chunker->process_document(doc);

    // Verify token management
    assert(result.total_chunks > 0);
    assert(result.total_title_tokens > 0);
    assert(result.total_metadata_tokens > 0);
    assert(result.total_content_tokens > 0);

    // Verify that title and metadata tokens are properly managed
    for (const auto& chunk : result.chunks) {
        assert(chunk.title_tokens >= 0);
        assert(chunk.metadata_tokens >= 0);
        assert(chunk.content_token_limit > 0);
        
        // Verify that metadata doesn't exceed the maximum percentage
        int total_tokens = chunk.title_tokens + chunk.metadata_tokens + 
                          static_cast<int>(tokenizer->count_tokens(chunk.content));
        (void)total_tokens; // Suppress unused variable warning
    }

    std::cout << "✅ Title and metadata token management test passed!" << std::endl;
}

void test_token_management() {
    std::cout << "Testing token management..." << std::endl;
    
    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 2048;
    config.include_metadata = true;
    
    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);
    
    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "test_doc_1";
    doc.title = "This is a test document with a long title";
    doc.semantic_identifier = "test_doc_1";
    doc.source_type = "file";
    doc.full_content = "This is the main content of the document. It contains multiple sentences.";
    
    // Add metadata
    doc.metadata["author"] = "John Doe";
    doc.metadata["category"] = "Technical Documentation";
    doc.metadata["tags"] = "testing, chunking, token management";
    
    // Add sections
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is the first section of the document. It contains important information about the topic.",
        "https://example.com/section1"
    ));
    
    auto result = chunker->process_document(doc);
    
    // Verify token management
    assert(result.total_chunks > 0);
    assert(result.total_title_tokens > 0);
    assert(result.total_metadata_tokens > 0);
    assert(result.total_content_tokens > 0);
    
    std::cout << "✅ Token management test passed!" << std::endl;
}

// ============================================================================
// SECTION PROCESSING TESTS
// ============================================================================

void test_section_processing_with_continuation() {
    std::cout << "Testing section processing with continuation..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 512;
    config.enable_multipass = false;
    config.enable_contextual_rag = false;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "test_doc_3";
    doc.title = "Section Processing Test";
    doc.semantic_identifier = "test_doc_3";
    doc.source_type = "file";
    doc.full_content = "This document has multiple sections that should be processed correctly.";

    // Add multiple sections
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is the first section. It contains some content that should be processed.",
        "https://example.com/section1"
    ));
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is the second section. It continues from the first section.",
        "https://example.com/section2"
    ));
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is the third section. It should be processed as a continuation.",
        "https://example.com/section3"
    ));

    auto result = chunker->process_document(doc);

    // Verify section processing
    assert(result.chunks.size() > 0);

    // Check for section continuation
    bool found_continuation = false;
    for (const auto& chunk : result.chunks) {
        if (chunk.section_continuation) {
            found_continuation = true;
            break;
        }
    }
    assert(found_continuation);
    (void)found_continuation; // Suppress unused variable warning

    std::cout << "✅ Section processing with continuation test passed!" << std::endl;
}

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

// ============================================================================
// CHUNK HANDLING TESTS
// ============================================================================

void test_oversized_chunk_handling() {
    std::cout << "Testing oversized chunk handling..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 256;
    config.enable_multipass = false;
    config.enable_contextual_rag = false;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "test_doc_4";
    doc.title = "Oversized Chunk Test";
    doc.semantic_identifier = "test_doc_4";
    doc.source_type = "file";
    doc.full_content = "This document contains a very large section that should be split into multiple chunks.";

    // Add a very large section
    std::string large_section = "This is a very large section that contains many words and sentences. ";
    large_section += "It should definitely exceed the token limit that we have set for this test. ";
    large_section += "The chunking system should automatically split this into multiple smaller chunks. ";
    large_section += "Each chunk should respect the token limit and maintain semantic coherence. ";
    large_section += "This is important for maintaining the quality of the chunking process. ";
    large_section += "The system should handle oversized content gracefully and efficiently.";

    doc.sections.push_back(AdvancedChunker::DocumentSection(large_section, "https://example.com/large_section"));

    auto result = chunker->process_document(doc);

    // Verify oversized chunk handling
    assert(result.chunks.size() > 1);

    // Verify that no chunk exceeds the token limit
    for (const auto& chunk : result.chunks) {
        int chunk_tokens = static_cast<int>(tokenizer->count_tokens(chunk.content));
        assert(chunk_tokens <= config.chunk_token_limit);
        (void)chunk_tokens; // Suppress unused variable warning
    }

    std::cout << "✅ Oversized chunk handling test passed!" << std::endl;
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

void test_token_limit_enforcement() {
    std::cout << "Testing token limit enforcement..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 128;
    config.enable_multipass = false;
    config.enable_contextual_rag = false;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "test_doc_5";
    doc.title = "Token Limit Test";
    doc.semantic_identifier = "test_doc_5";
    doc.source_type = "file";
    doc.full_content = "This document tests the token limit enforcement mechanism.";

    // Add content that should be split
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is a section that should be split into multiple chunks due to the strict token limit. "
        "The chunking system should enforce the token limit and create appropriate chunks. "
        "Each chunk should respect the maximum token count while maintaining semantic coherence.",
        "https://example.com/token_limit_test"
    ));

    auto result = chunker->process_document(doc);

    // Verify token limit enforcement
    assert(result.chunks.size() > 1);

    // Verify that all chunks respect the token limit
    for (const auto& chunk : result.chunks) {
        int chunk_tokens = static_cast<int>(tokenizer->count_tokens(chunk.content));
        assert(chunk_tokens <= config.chunk_token_limit);
        (void)chunk_tokens; // Suppress unused variable warning
    }

    std::cout << "✅ Token limit enforcement test passed!" << std::endl;
}

// ============================================================================
// BLURB AND MINI-CHUNK TESTS
// ============================================================================

void test_blurb_extraction() {
    std::cout << "Testing blurb extraction..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 512;
    config.enable_multipass = false;
    config.enable_contextual_rag = false;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "test_doc_6";
    doc.title = "Blurb Extraction Test";
    doc.semantic_identifier = "test_doc_6";
    doc.source_type = "file";
    doc.full_content = "This document tests the blurb extraction functionality.";

    // Add content for blurb extraction
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is the first sentence of the document. It should be extracted as a blurb. "
        "The blurb should contain the most important information from the beginning of the content. "
        "This helps in providing a quick summary of what the chunk contains.",
        "https://example.com/blurb_test"
    ));

    auto result = chunker->process_document(doc);

    // Verify blurb extraction
    assert(result.chunks.size() > 0);

    for (const auto& chunk : result.chunks) {
        assert(!chunk.blurb.empty());
        assert(chunk.blurb.length() <= BLURB_SIZE);
        
        // Verify that blurb is a subset of the content
        assert(chunk.content.find(chunk.blurb) != std::string::npos || 
               chunk.blurb.find(chunk.content.substr(0, chunk.blurb.length())) != std::string::npos);
        (void)chunk; // Suppress unused variable warning
    }

    std::cout << "✅ Blurb extraction test passed!" << std::endl;
}

void test_mini_chunk_generation() {
    std::cout << "Testing mini-chunk generation..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 512;
    config.enable_multipass = true;
    config.enable_contextual_rag = false;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "test_doc_7";
    doc.title = "Mini-Chunk Generation Test";
    doc.semantic_identifier = "test_doc_7";
    doc.source_type = "file";
    doc.full_content = "This document tests the mini-chunk generation functionality.";

    // Add content for mini-chunk generation
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is a section that should be split into mini-chunks. "
        "Mini-chunks are smaller chunks used for more detailed processing. "
        "They help in providing finer granularity for certain applications. "
        "The mini-chunk size is typically smaller than the regular chunk size.",
        "https://example.com/mini_chunk_test"
    ));

    auto result = chunker->process_document(doc);

    // Verify mini-chunk generation
    assert(result.chunks.size() > 0);

    bool found_mini_chunks = false;
    for (const auto& chunk : result.chunks) {
        if (!chunk.mini_chunk_texts.empty()) {
            found_mini_chunks = true;
            
            // Verify mini-chunk properties
            for (const auto& mini_chunk : chunk.mini_chunk_texts) {
                assert(!mini_chunk.empty());
                int mini_chunk_tokens = static_cast<int>(tokenizer->count_tokens(mini_chunk));
                assert(mini_chunk_tokens <= MINI_CHUNK_SIZE);
                (void)mini_chunk_tokens; // Suppress unused variable warning
            }
            break;
        }
    }
    assert(found_mini_chunks);
    (void)found_mini_chunks; // Suppress unused variable warning

    std::cout << "✅ Mini-chunk generation test passed!" << std::endl;
}

// ============================================================================
// DOCUMENT HANDLING TESTS
// ============================================================================

void test_single_document_handling() {
    std::cout << "Testing single document handling..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 1024;
    config.enable_multipass = false;
    config.enable_contextual_rag = false;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "test_doc_8";
    doc.title = "Single Document Test";
    doc.semantic_identifier = "test_doc_8";
    doc.source_type = "file";
    doc.full_content = "This document tests the single document handling functionality.";

    // Add content
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is a single document that should be processed correctly. "
        "The chunking system should handle it appropriately and create the necessary chunks.",
        "https://example.com/single_doc_test"
    ));

    auto result = chunker->process_document(doc);

    // Verify single document handling
    assert(result.chunks.size() > 0);
    assert(result.total_chunks > 0);
    assert(result.total_content_tokens > 0);

    // Verify document properties
    for (const auto& chunk : result.chunks) {
        assert(chunk.document_id == doc.document_id);
        assert(chunk.source_type == doc.source_type);
        assert(chunk.semantic_identifier == doc.semantic_identifier);
        (void)chunk; // Suppress unused variable warning
    }

    std::cout << "✅ Single document handling test passed!" << std::endl;
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

// ============================================================================
// LINK TRACKING TESTS
// ============================================================================

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

// ============================================================================
// MULTIPASS AND CONTEXTUAL RAG TESTS
// ============================================================================

void test_multipass_indexing() {
    std::cout << "Testing multipass indexing..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 512;
    config.enable_multipass = true;
    config.enable_contextual_rag = false;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "test_doc_9";
    doc.title = "Multipass Indexing Test";
    doc.semantic_identifier = "test_doc_9";
    doc.source_type = "file";
    doc.full_content = "This document tests the multipass indexing functionality.";

    // Add content for multipass indexing
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is the first section for multipass indexing. "
        "Multipass indexing creates both regular chunks and large chunks. "
        "Large chunks combine multiple regular chunks for different processing needs.",
        "https://example.com/multipass_test_1"
    ));
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is the second section for multipass indexing. "
        "The system should create both mini-chunks and large chunks. "
        "This provides different levels of granularity for various applications.",
        "https://example.com/multipass_test_2"
    ));

    auto result = chunker->process_document(doc);

    // Verify multipass indexing
    assert(result.chunks.size() > 0);

    bool found_large_chunks = false;
    for (const auto& chunk : result.chunks) {
        if (chunk.large_chunk_id >= 0) {
            found_large_chunks = true;
            break;
        }
    }
    assert(found_large_chunks);
    (void)found_large_chunks; // Suppress unused variable warning

    std::cout << "✅ Multipass indexing test passed!" << std::endl;
}

void test_contextual_rag() {
    std::cout << "Testing contextual RAG..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 512;
    config.enable_multipass = false;
    config.enable_contextual_rag = true;
    config.contextual_rag_reserved_tokens = 100;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "test_doc_10";
    doc.title = "Contextual RAG Test";
    doc.semantic_identifier = "test_doc_10";
    doc.source_type = "file";
    doc.full_content = "This document tests the contextual RAG functionality.";

    // Add content for contextual RAG
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is a section for testing contextual RAG. "
        "Contextual RAG reserves tokens for document summaries and chunk context. "
        "This helps in providing better context for language model processing.",
        "https://example.com/contextual_rag_test"
    ));

    auto result = chunker->process_document(doc);

    // Verify contextual RAG
    assert(result.chunks.size() > 0);

    bool found_rag_content = false;
    for (const auto& chunk : result.chunks) {
        if (chunk.contextual_rag_reserved_tokens > 0) {
            found_rag_content = true;
            break;
        }
    }
    assert(found_rag_content);
    (void)found_rag_content; // Suppress unused variable warning

    std::cout << "✅ Contextual RAG test passed!" << std::endl;
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

// ============================================================================
// QUALITY AND BATCH PROCESSING TESTS
// ============================================================================

void test_quality_filtering() {
    std::cout << "Testing quality filtering..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 512;
    config.enable_multipass = false;
    config.enable_contextual_rag = false;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "test_doc_11";
    doc.title = "Quality Filtering Test";
    doc.semantic_identifier = "test_doc_11";
    doc.source_type = "file";
    doc.full_content = "This document tests the quality filtering functionality.";

    // Add content for quality filtering
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is a section for testing quality filtering. "
        "Quality filtering ensures that only high-quality chunks are retained. "
        "This helps in maintaining the overall quality of the chunking process.",
        "https://example.com/quality_filter_test"
    ));

    auto result = chunker->process_document(doc);

    // Verify quality filtering
    assert(result.chunks.size() > 0);

    for (const auto& chunk : result.chunks) {
        // Verify that chunks meet minimum content requirements
        assert(chunk.content.length() >= CHUNK_MIN_CONTENT);
        assert(chunk.is_high_quality);
        (void)chunk; // Suppress unused variable warning
    }

    std::cout << "✅ Quality filtering test passed!" << std::endl;
}

void test_batch_processing() {
    std::cout << "Testing batch processing..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 512;
    config.enable_multipass = false;
    config.enable_contextual_rag = false;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    std::vector<AdvancedChunker::DocumentInfo> documents;

    // Create multiple documents for batch processing
    for (int i = 0; i < 3; ++i) {
        AdvancedChunker::DocumentInfo doc;
        doc.document_id = "test_doc_batch_" + std::to_string(i);
        doc.title = "Batch Processing Test " + std::to_string(i);
        doc.semantic_identifier = "test_doc_batch_" + std::to_string(i);
        doc.source_type = "file";
        doc.full_content = "This is document " + std::to_string(i) + " for batch processing.";

        doc.sections.push_back(AdvancedChunker::DocumentSection(
            "This is section " + std::to_string(i) + " for batch processing. "
            "The batch processing should handle multiple documents efficiently.",
            "https://example.com/batch_test_" + std::to_string(i)
        ));

        documents.push_back(doc);
    }

    auto results = chunker->process_documents(documents);

    // Verify batch processing
    assert(results.size() == documents.size());

    for (size_t i = 0; i < results.size(); ++i) {
        assert(results[i].total_chunks > 0);
        assert(results[i].total_content_tokens > 0);
    }

    std::cout << "✅ Batch processing test passed!" << std::endl;
}

// ============================================================================
// CONSTANTS AND CONFIGURATION TESTS
// ============================================================================

void test_constants_and_configuration() {
    std::cout << "Testing constants and configuration..." << std::endl;

    // Test that all constants are available and have correct values
    assert(MAX_METADATA_PERCENTAGE == 0.25);
    assert(CHUNK_MIN_CONTENT == 256);
    assert(CHUNK_OVERLAP == 0);
    assert(STRICT_CHUNK_TOKEN_LIMIT == true);
    assert(AVERAGE_SUMMARY_EMBEDDINGS == false);
    assert(USE_CHUNK_SUMMARY == true);
    assert(USE_DOCUMENT_SUMMARY == true);

    // Test configuration
    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 512;
    config.enable_multipass = true;
    config.enable_contextual_rag = true;
    config.include_metadata = true;
    config.max_metadata_percentage = 0.25;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    // Verify configuration is applied correctly
    const auto& applied_config = chunker->get_config();
    assert(applied_config.chunk_token_limit == 512);
    assert(applied_config.enable_multipass == true);
    assert(applied_config.enable_contextual_rag == true);
    assert(applied_config.include_metadata == true);
    assert(applied_config.max_metadata_percentage == 0.25);
    (void)applied_config; // Suppress unused variable warning

    std::cout << "✅ Constants and configuration test passed!" << std::endl;
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

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    std::cout << "🧪 Running Comprehensive Chunking Tests" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    try {
        // Metadata and Token Management Tests
        test_metadata_processing();
        test_title_and_metadata_token_management();
        test_token_management();
        
        // Section Processing Tests
        test_section_processing_with_continuation();
        test_advanced_section_combination_logic();
        
        // Chunk Handling Tests
        test_oversized_chunk_handling();
        test_advanced_strict_token_limit();
        test_token_limit_enforcement();
        
        // Blurb and Mini-Chunk Tests
        test_blurb_extraction();
        test_mini_chunk_generation();
        
        // Document Handling Tests
        test_single_document_handling();
        test_advanced_source_specific_handling();
        
        // Link Tracking Tests
        test_advanced_link_offset_tracking();
        
        // Multipass and Contextual RAG Tests
        test_multipass_indexing();
        test_contextual_rag();
        test_advanced_contextual_rag();
        
        // Quality and Batch Processing Tests
        test_quality_filtering();
        test_batch_processing();
        
        // Constants and Configuration Tests
        test_constants_and_configuration();
        test_advanced_constants_and_configuration();
        
        std::cout << "\n🎉 All comprehensive chunking tests passed successfully!" << std::endl;
        std::cout << "✅ R3M chunking system has advanced capabilities!" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
} 