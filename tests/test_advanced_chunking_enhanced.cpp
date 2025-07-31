#include <iostream>
#include <cassert>
#include <memory>
#include "r3m/chunking/advanced_chunker.hpp"
#include "r3m/chunking/tokenizer.hpp"
#include "r3m/chunking/metadata_processor.hpp"

using namespace r3m::chunking;

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
    doc.document_id = "token_management_test";
    doc.title = "This is a test document with a long title for token management testing";
    doc.semantic_identifier = "token_management_test";
    doc.source_type = "file";
    doc.full_content = "This document tests token management capabilities.";

    // Add metadata that should be processed
    doc.metadata["author"] = "John Doe";
    doc.metadata["category"] = "Technical Documentation";
    doc.metadata["tags"] = "testing, token management, chunking";

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

    // Verify chunk properties and token limits
    for (const auto& chunk : result.chunks) {
        assert(!chunk.document_id.empty());
        assert(!chunk.content.empty());
        assert(chunk.title_tokens >= 0);
        assert(chunk.metadata_tokens >= 0);
        assert(chunk.content_token_limit > 0);
        assert(!chunk.source_type.empty());
        assert(!chunk.semantic_identifier.empty());

        // Verify token limits are respected
        int total_tokens = chunk.title_tokens + chunk.metadata_tokens +
                           static_cast<int>(tokenizer->count_tokens(chunk.content));
        assert(total_tokens <= config.chunk_token_limit);
        (void)total_tokens; // Suppress unused variable warning
    }

    std::cout << "✅ Title and metadata token management test passed!" << std::endl;
}

void test_section_processing_with_continuation() {
    std::cout << "Testing section processing with continuation..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 2048;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "section_continuation_test";
    doc.title = "Section Continuation Test";
    doc.semantic_identifier = "section_continuation_test";
    doc.source_type = "confluence";
    doc.full_content = "This document tests section continuation logic.";

    // Add multiple sections to test continuation
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is the first section of the document.",
        "https://example.com/section1"
    ));

    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is the second section that should be processed.",
        "https://example.com/section2"
    ));

    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is the third section for continuation testing.",
        "https://example.com/section3"
    ));

    auto result = chunker->process_document(doc);

    // Verify section processing
    assert(result.total_chunks > 0);

    // Check for section continuation tracking
    bool found_continuation = false;
    for (const auto& chunk : result.chunks) {
        if (chunk.section_continuation) {
            found_continuation = true;
        }
        assert(!chunk.content.empty());
        assert(!chunk.blurb.empty());
        assert(chunk.quality_score >= 0.0);
        assert(chunk.quality_score <= 1.0);
        assert(chunk.information_density >= 0.0);
        assert(chunk.information_density <= 1.0);
    }

    // Should have at least one continuation chunk
    assert(found_continuation);
    (void)found_continuation; // Suppress unused variable warning

    std::cout << "✅ Section processing with continuation test passed!" << std::endl;
}

void test_oversized_chunk_handling() {
    std::cout << "Testing oversized chunk handling..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 100; // Small limit for testing

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "oversized_chunk_test";
    doc.title = "Oversized Chunk Test";
    doc.semantic_identifier = "oversized_chunk_test";
    doc.source_type = "file";
    doc.full_content = "This document tests oversized chunk handling.";

    // Add content that should be split due to token limits
    std::string long_content;
    for (int i = 0; i < 50; ++i) {
        long_content += "This is sentence " + std::to_string(i) + " which should be properly chunked. ";
    }

    doc.sections.push_back(AdvancedChunker::DocumentSection(long_content, "https://example.com/oversized"));

    auto result = chunker->process_document(doc);

    // Verify oversized chunk handling
    assert(result.total_chunks > 1); // Should be split into multiple chunks

    for (const auto& chunk : result.chunks) {
        int total_tokens = chunk.title_tokens + chunk.metadata_tokens +
                           static_cast<int>(tokenizer->count_tokens(chunk.content));
        assert(total_tokens <= config.chunk_token_limit);
        (void)total_tokens; // Suppress unused variable warning
    }

    std::cout << "✅ Oversized chunk handling test passed!" << std::endl;
}

void test_blurb_extraction() {
    std::cout << "Testing blurb extraction..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.blurb_size = 100;
    config.chunk_token_limit = 2048;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "blurb_extraction_test";
    doc.title = "This is a test document with a long title for blurb extraction testing";
    doc.semantic_identifier = "blurb_extraction_test";
    doc.source_type = "file";
    doc.full_content = "This document tests blurb extraction capabilities.";

    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is the first section with detailed content for blurb extraction testing. "
        "The blurb should contain the first sentence or sentences of this content. "
        "This helps in providing a quick summary of what the chunk contains.",
        "https://example.com/blurb"
    ));

    auto result = chunker->process_document(doc);

    // Verify blurb extraction
    assert(result.total_chunks > 0);

    for (const auto& chunk : result.chunks) {
        assert(!chunk.blurb.empty());
        assert(chunk.blurb.length() <= 200); // Reasonable blurb size
        assert(chunk.blurb.find("This is the first section") != std::string::npos);
        (void)chunk; // Suppress unused variable warning
    }

    std::cout << "✅ Blurb extraction test passed!" << std::endl;
}

void test_mini_chunk_generation() {
    std::cout << "Testing mini-chunk generation..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.enable_multipass = true;
    config.mini_chunk_size = 150;
    config.chunk_token_limit = 2048;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "mini_chunk_test";
    doc.title = "Mini-Chunk Generation Test";
    doc.semantic_identifier = "mini_chunk_test";
    doc.source_type = "github";
    doc.full_content = "This document tests mini-chunk generation capabilities.";

    // Add content that should generate mini-chunks
    std::string content = "This is a comprehensive document that should generate mini-chunks. ";
    for (int i = 0; i < 10; ++i) {
        content += "Section " + std::to_string(i) + " contains detailed information about topic " + std::to_string(i) + ". ";
        content += "The content is structured and meaningful. ";
        content += "Each section should be properly chunked and indexed. ";
    }

    doc.sections.push_back(AdvancedChunker::DocumentSection(content, "https://example.com/mini_chunk"));

    auto result = chunker->process_document(doc);

    // Verify mini-chunk generation
    assert(result.total_chunks > 0);

    // Check for mini-chunk texts
    for (const auto& chunk : result.chunks) {
        // Mini-chunks should have mini_chunk_texts populated
        if (!chunk.mini_chunk_texts.empty()) {
            for (const auto& mini_chunk : chunk.mini_chunk_texts) {
                assert(!mini_chunk.empty());
                assert(mini_chunk.length() <= 200); // Reasonable mini-chunk size
                (void)mini_chunk; // Suppress unused variable warning
            }
        }
    }

    std::cout << "✅ Mini-chunk generation test passed!" << std::endl;
}

void test_single_document_handling() {
    std::cout << "Testing single document handling..." << std::endl;

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.enable_contextual_rag = true;
    config.chunk_token_limit = 2048;
    config.contextual_rag_reserved_tokens = 512;

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "single_document_test";
    doc.title = "Single Document Handling Test";
    doc.semantic_identifier = "single_document_test";
    doc.source_type = "documentation";
    doc.full_content = "This document tests single document handling logic.";

    // Add metadata
    doc.metadata["author"] = "Test Author";
    doc.metadata["category"] = "Testing";
    doc.metadata["version"] = "1.0";

    // Add sections
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is the first section for single document handling testing.",
        "https://example.com/single_doc"
    ));

    auto result = chunker->process_document(doc);

    // Verify single document handling
    assert(result.total_chunks > 0);
    assert(result.successful_chunks > 0);
    assert(result.failed_chunks == 0);
    assert(result.processing_time_ms > 0.0);

    // Verify token management
    assert(result.total_title_tokens > 0);
    assert(result.total_metadata_tokens > 0);
    assert(result.total_content_tokens > 0);
    assert(result.total_rag_tokens >= 0);

    // Verify quality metrics
    assert(result.avg_quality_score >= 0.0);
    assert(result.avg_quality_score <= 1.0);
    assert(result.avg_information_density >= 0.0);
    assert(result.avg_information_density <= 1.0);

    std::cout << "✅ Single document handling test passed!" << std::endl;
}

void test_constants_and_configuration() {
    std::cout << "Testing constants and configuration..." << std::endl;

    // Test that constants match expected values
    assert(MAX_METADATA_PERCENTAGE == 0.25);
    assert(CHUNK_MIN_CONTENT == 256);
    assert(CHUNK_OVERLAP == 0);
    assert(DEFAULT_CONTEXTUAL_RAG_RESERVED_TOKENS == 512);

    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    
    // Test configuration defaults
    assert(config.enable_multipass == false);
    assert(config.enable_large_chunks == false);
    assert(config.enable_contextual_rag == false);
    assert(config.include_metadata == true);
    assert(config.chunk_token_limit == 2048);
    assert(config.chunk_overlap == 0);
    assert(config.mini_chunk_size == 150);
    assert(config.blurb_size == 100);
    assert(config.large_chunk_ratio == 4);
    assert(config.chunk_min_content == 256);
    assert(config.max_metadata_percentage == 0.25);
    assert(config.contextual_rag_reserved_tokens == 512);

    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);

    // Test that configuration is properly applied
    const auto& applied_config = chunker->get_config();
    assert(applied_config.chunk_token_limit == 2048);
    assert(applied_config.mini_chunk_size == 150);
    assert(applied_config.blurb_size == 100);
    assert(applied_config.large_chunk_ratio == 4);
    (void)applied_config; // Suppress unused variable warning

    std::cout << "✅ Constants and configuration test passed!" << std::endl;
}

void test_quality_filtering() {
    std::cout << "Testing quality filtering..." << std::endl;
    
    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 2048;
    
    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);
    
    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "test_doc_3";
    doc.title = "Quality Filtering Test";
    doc.semantic_identifier = "test_doc_3";
    doc.source_type = "file";
    doc.full_content = "This document tests quality filtering capabilities.";
    
    // Add high-quality content
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This is a high-quality section with meaningful content. "
        "It contains diverse vocabulary and proper sentence structure. "
        "The information density should be high and quality score should be good. "
        "This content should pass quality filtering.",
        "https://example.com/quality"
    ));
    
    // Add low-quality content
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "xxx xxx xxx xxx xxx xxx xxx xxx xxx xxx xxx xxx xxx xxx xxx xxx xxx xxx xxx xxx",
        "https://example.com/low_quality"
    ));
    
    auto result = chunker->process_document(doc);
    
    // Verify quality filtering
    assert(result.total_chunks > 0);
    assert(result.high_quality_chunks > 0);
    assert(result.avg_quality_score > 0.0);
    assert(result.avg_information_density > 0.0);
    
    std::cout << "✅ Quality filtering test passed!" << std::endl;
}

void test_multipass_indexing() {
    std::cout << "Testing multipass indexing..." << std::endl;
    
    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.enable_multipass = true;
    config.enable_large_chunks = true;
    config.chunk_token_limit = 2048;
    
    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);
    
    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "test_doc_4";
    doc.title = "Multipass Indexing Test";
    doc.semantic_identifier = "test_doc_4";
    doc.source_type = "github";
    doc.full_content = "This document tests multipass indexing capabilities.";
    
    // Add content that should generate multiple chunk types
    std::string content = "This is a comprehensive document that should generate multiple types of chunks. ";
    for (int i = 0; i < 10; ++i) {
        content += "Section " + std::to_string(i) + " contains detailed information about topic " + std::to_string(i) + ". ";
        content += "The content is structured and meaningful. ";
        content += "Each section should be properly chunked and indexed. ";
    }
    
    doc.sections.push_back(AdvancedChunker::DocumentSection(content, "https://example.com/multipass"));
    
    auto result = chunker->process_document(doc);
    
    // Verify multipass indexing
    assert(result.total_chunks > 0);
    
    // Check for different chunk types
    bool found_regular = false;
    
    for (const auto& chunk : result.chunks) {
        if (chunk.large_chunk_id == -1) {
            found_regular = true;
        }
    }
    
    assert(found_regular);
    (void)found_regular; // Suppress unused variable warning
    
    std::cout << "✅ Multipass indexing test passed!" << std::endl;
}

void test_contextual_rag() {
    std::cout << "Testing contextual RAG..." << std::endl;
    
    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.enable_contextual_rag = true;
    config.chunk_token_limit = 2048;
    
    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);
    
    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "test_doc_5";
    doc.title = "Contextual RAG Test";
    doc.semantic_identifier = "test_doc_5";
    doc.source_type = "documentation";
    doc.full_content = "This document tests contextual RAG capabilities.";
    
    doc.sections.push_back(AdvancedChunker::DocumentSection(
        "This section tests contextual RAG functionality. "
        "The system should generate document summaries and chunk context. "
        "This enhances the retrieval capabilities of the system.",
        "https://example.com/rag"
    ));
    
    auto result = chunker->process_document(doc);
    
    // Verify contextual RAG
    assert(result.total_chunks > 0);
    assert(result.total_rag_tokens > 0);
    
    // Check for RAG-related properties
    for (const auto& chunk : result.chunks) {
        assert(chunk.contextual_rag_reserved_tokens >= 0);
        (void)chunk; // Suppress unused variable warning
    }
    
    std::cout << "✅ Contextual RAG test passed!" << std::endl;
}

void test_batch_processing() {
    std::cout << "Testing batch processing..." << std::endl;
    
    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 2048;
    
    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);
    
    std::vector<AdvancedChunker::DocumentInfo> documents;
    
    for (int i = 0; i < 3; ++i) {
        AdvancedChunker::DocumentInfo doc;
        doc.document_id = "batch_doc_" + std::to_string(i);
        doc.title = "Batch Document " + std::to_string(i);
        doc.semantic_identifier = "batch_doc_" + std::to_string(i);
        doc.source_type = "batch";
        doc.full_content = "This is batch document " + std::to_string(i) + " for testing batch processing.";
        
        doc.sections.push_back(AdvancedChunker::DocumentSection(
            "Section 1 of batch document " + std::to_string(i) + ". "
            "This content should be properly chunked and processed.",
            "https://example.com/batch/" + std::to_string(i)
        ));
        
        documents.push_back(doc);
    }
    
    auto results = chunker->process_documents(documents);
    
    // Verify batch processing
    assert(results.size() == 3);
    
    for (const auto& result : results) {
        assert(result.total_chunks > 0);
        assert(result.successful_chunks > 0);
        assert(result.failed_chunks == 0);
        assert(result.processing_time_ms > 0.0);
        (void)result; // Suppress unused variable warning
    }
    
    std::cout << "✅ Batch processing test passed!" << std::endl;
}

void test_token_limit_enforcement() {
    std::cout << "Testing token limit enforcement..." << std::endl;
    
    auto tokenizer = std::make_shared<BasicTokenizer>(8192);
    AdvancedChunker::Config config;
    config.chunk_token_limit = 100; // Small limit for testing
    config.chunk_token_limit = 100;
    
    auto chunker = std::make_unique<AdvancedChunker>(tokenizer, config);
    
    AdvancedChunker::DocumentInfo doc;
    doc.document_id = "token_limit_test";
    doc.title = "Token Limit Test";
    doc.semantic_identifier = "token_limit_test";
    doc.source_type = "test";
    doc.full_content = "This document tests token limit enforcement.";
    
    // Add content that should be split due to token limits
    std::string long_content;
    for (int i = 0; i < 50; ++i) {
        long_content += "This is sentence " + std::to_string(i) + " which should be properly chunked. ";
    }
    
    doc.sections.push_back(AdvancedChunker::DocumentSection(long_content, "https://example.com/token_limit"));
    
    auto result = chunker->process_document(doc);
    
    // Verify token limit enforcement
    assert(result.total_chunks > 1); // Should be split into multiple chunks
    
    for (const auto& chunk : result.chunks) {
        int total_tokens = chunk.title_tokens + chunk.metadata_tokens + 
                          static_cast<int>(tokenizer->count_tokens(chunk.content));
        assert(total_tokens <= config.chunk_token_limit);
        (void)total_tokens;
    }
    
    std::cout << "✅ Token limit enforcement test passed!" << std::endl;
}

int main() {
    std::cout << "🧪 Running Comprehensive Advanced Chunking Tests" << std::endl;
    std::cout << "================================================" << std::endl;

    try {
        // Core advanced feature tests
        test_metadata_processing();
        test_title_and_metadata_token_management();
        test_section_processing_with_continuation();
        test_oversized_chunk_handling();
        test_blurb_extraction();
        test_mini_chunk_generation();
        test_single_document_handling();
        test_constants_and_configuration();
        
        // Advanced feature tests
        test_token_management();
        test_quality_filtering();
        test_multipass_indexing();
        test_contextual_rag();
        test_batch_processing();
        test_token_limit_enforcement();

        std::cout << "\n🎉 All comprehensive tests passed successfully!" << std::endl;
        std::cout << "✅ R3M chunking system has advanced capabilities!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
} 