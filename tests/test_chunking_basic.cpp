#include "r3m/chunking/tokenizer.hpp"
#include "r3m/chunking/sentence_chunker.hpp"
#include "r3m/chunking/chunk_models.hpp"
#include <iostream>
#include <cassert>

using namespace r3m::chunking;

int main() {
    std::cout << "🧪 Testing R3M Chunking Components..." << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // Test 1: Basic Tokenizer
    std::cout << "\n📝 Testing Basic Tokenizer..." << std::endl;
    auto tokenizer = TokenizerFactory::create(TokenizerFactory::Type::BASIC, 1000);
    
    std::string test_text = "Hello world! This is a test sentence. It has multiple sentences.";
    auto tokens = tokenizer->tokenize(test_text);
    auto token_count = tokenizer->count_tokens(test_text);
    
    std::cout << "Text: " << test_text << std::endl;
    std::cout << "Token count: " << token_count << std::endl;
    std::cout << "Tokens: ";
    for (const auto& token : tokens) {
        std::cout << "[" << token << "] ";
    }
    std::cout << std::endl;
    
    assert(token_count > 0);
    assert(tokens.size() == token_count);
    std::cout << "✅ Basic Tokenizer test passed!" << std::endl;
    
    // Test 2: Sentence Chunker
    std::cout << "\n📄 Testing Sentence Chunker..." << std::endl;
    auto sentence_chunker = std::make_shared<SentenceChunker>(
        std::move(tokenizer), 15, 0, "texts"
    );
    
    std::string long_text = "This is the first sentence. This is the second sentence. "
                           "This is the third sentence. This is the fourth sentence. "
                           "This is the fifth sentence. This is the sixth sentence.";
    
    auto chunks = sentence_chunker->chunk(long_text);
    
    std::cout << "Long text: " << long_text << std::endl;
    std::cout << "Number of chunks: " << chunks.size() << std::endl;
    
    for (size_t i = 0; i < chunks.size(); ++i) {
        std::cout << "Chunk " << i + 1 << ": " << chunks[i] << std::endl;
        std::cout << "  About to count tokens..." << std::endl;
        auto test_tokenizer = TokenizerFactory::create(TokenizerFactory::Type::BASIC, 1000);
        auto chunk_tokens = test_tokenizer->count_tokens(chunks[i]);
        std::cout << "  Token count: " << chunk_tokens << std::endl;
        assert(chunk_tokens <= sentence_chunker->get_chunk_size());
    }
    
    assert(chunks.size() > 1);
    std::cout << "✅ Sentence Chunker test passed!" << std::endl;
    
    // Test 3: Chunk Models
    std::cout << "\n🏗️ Testing Chunk Models..." << std::endl;
    
    DocumentChunk chunk;
    chunk.chunk_id = 1;
    chunk.document_id = "test_doc_001";
    chunk.blurb = "This is the first sentence.";
    chunk.content = "This is the first sentence. This is the second sentence.";
    chunk.title_prefix = "Test Document";
    chunk.metadata_semantic = "Metadata: author - John Doe, date - 2024";
    chunk.metadata_keyword = "John Doe 2024";
    chunk.quality_score = 0.85;
    chunk.information_density = 0.72;
    chunk.is_high_quality = true;
    
    std::cout << "Chunk ID: " << chunk.chunk_id << std::endl;
    std::cout << "Document ID: " << chunk.document_id << std::endl;
    std::cout << "Blurb: " << chunk.blurb << std::endl;
    std::cout << "Content length: " << chunk.content.length() << " characters" << std::endl;
    std::cout << "Title prefix: " << chunk.title_prefix << std::endl;
    std::cout << "Quality score: " << chunk.quality_score << std::endl;
    std::cout << "Information density: " << chunk.information_density << std::endl;
    std::cout << "High quality: " << (chunk.is_high_quality ? "YES" : "NO") << std::endl;
    std::cout << "Short descriptor: " << chunk.to_short_descriptor() << std::endl;
    
    assert(chunk.chunk_id == 1);
    assert(chunk.document_id == "test_doc_001");
    assert(!chunk.blurb.empty());
    assert(!chunk.content.empty());
    assert(chunk.quality_score > 0.0);
    assert(chunk.information_density > 0.0);
    assert(chunk.is_high_quality);
    
    std::cout << "✅ Chunk Models test passed!" << std::endl;
    
    // Test 4: Chunking Result
    std::cout << "\n📊 Testing Chunking Result..." << std::endl;
    
    ChunkingResult result;
    result.chunks.push_back(chunk);
    result.total_chunks = 1;
    result.successful_chunks = 1;
    result.failed_chunks = 0;
    result.processing_time_ms = 15.5;
    result.avg_quality_score = 0.85;
    result.avg_information_density = 0.72;
    result.high_quality_chunks = 1;
    
    std::cout << "Total chunks: " << result.total_chunks << std::endl;
    std::cout << "Successful chunks: " << result.successful_chunks << std::endl;
    std::cout << "Failed chunks: " << result.failed_chunks << std::endl;
    std::cout << "Processing time: " << result.processing_time_ms << " ms" << std::endl;
    std::cout << "Average quality score: " << result.avg_quality_score << std::endl;
    std::cout << "Average information density: " << result.avg_information_density << std::endl;
    std::cout << "High quality chunks: " << result.high_quality_chunks << std::endl;
    
    assert(result.total_chunks == 1);
    assert(result.successful_chunks == 1);
    assert(result.failed_chunks == 0);
    assert(result.processing_time_ms > 0.0);
    assert(result.avg_quality_score > 0.0);
    assert(result.avg_information_density > 0.0);
    assert(result.high_quality_chunks == 1);
    
    std::cout << "✅ Chunking Result test passed!" << std::endl;
    
    std::cout << "\n🎉 All chunking component tests passed!" << std::endl;
    std::cout << "✅ Basic tokenizer working" << std::endl;
    std::cout << "✅ Sentence chunker working" << std::endl;
    std::cout << "✅ Chunk models working" << std::endl;
    std::cout << "✅ Chunking result working" << std::endl;
    
    return 0;
} 