#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "r3m/utils/text_processing.hpp"
#include "r3m/chunking/advanced_tokenizer.hpp"

using namespace r3m::utils;
using namespace r3m::chunking;

void test_text_processing_utilities() {
    std::cout << "Testing text processing utilities..." << std::endl;
    
    // Test clean_text
    std::string dirty_text = "Hello\u0000World\u0001\u0002Test";
    std::string cleaned = TextProcessing::clean_text(dirty_text);
    assert(cleaned == "HelloWorldTest");
    std::cout << "✅ clean_text test passed!" << std::endl;
    
    // Test shared_precompare_cleanup
    std::string text_with_formatting = "Hello, World! *Bold* \"Quote\" #hashtag";
    std::string cleaned_for_comparison = TextProcessing::shared_precompare_cleanup(text_with_formatting);
    assert(cleaned_for_comparison == "helloworld");
    std::cout << "✅ shared_precompare_cleanup test passed!" << std::endl;
    
    // Test remove_punctuation
    std::string text_with_punct = "Hello, World! How are you?";
    std::string without_punct = TextProcessing::remove_punctuation(text_with_punct);
    assert(without_punct == "Hello World How are you");
    std::cout << "✅ remove_punctuation test passed!" << std::endl;
    
    // Test replace_whitespaces_with_space
    std::string text_with_whitespace = "Hello\tWorld\nTest\r\nMore";
    std::string normalized = TextProcessing::replace_whitespaces_with_space(text_with_whitespace);
    assert(normalized == "Hello World Test More");
    std::cout << "✅ replace_whitespaces_with_space test passed!" << std::endl;
    
    // Test escape_newlines
    std::string text_with_newlines = "Line1\nLine2\nLine3";
    std::string escaped = TextProcessing::escape_newlines(text_with_newlines);
    assert(escaped == "Line1\\\\nLine2\\\\nLine3");
    std::cout << "✅ escape_newlines test passed!" << std::endl;
    
    // Test make_url_compatible
    std::string text_for_url = "Hello World Test";
    std::string url_safe = TextProcessing::make_url_compatible(text_for_url);
    assert(url_safe.find("Hello_World_Test") != std::string::npos);
    std::cout << "✅ make_url_compatible test passed!" << std::endl;
    
    // Test has_unescaped_quote
    std::string text_with_quotes = "Hello \"World\" Test";
    assert(TextProcessing::has_unescaped_quote(text_with_quotes) == true);
    std::string text_with_escaped_quotes = "Hello \\\"World\\\" Test";
    assert(TextProcessing::has_unescaped_quote(text_with_escaped_quotes) == false);
    std::cout << "✅ has_unescaped_quote test passed!" << std::endl;
    
    // Test escape_quotes
    std::string json_text = "{\"key\": \"value\", \"test\": \"data\"}";
    std::string escaped_json = TextProcessing::escape_quotes(json_text);
    assert(escaped_json.find("\\\"") != std::string::npos);
    std::cout << "✅ escape_quotes test passed!" << std::endl;
    
    // Test clean_up_code_blocks
    std::string code_block = "```\nint main() {\n    return 0;\n}\n```";
    std::string cleaned_code = TextProcessing::clean_up_code_blocks(code_block);
    assert(cleaned_code == "int main() {\n    return 0;\n}");
    std::cout << "✅ clean_up_code_blocks test passed!" << std::endl;
    
    // Test clean_model_quote
    std::string quoted_text = "\"Hello World\"";
    std::string cleaned_quote = TextProcessing::clean_model_quote(quoted_text);
    assert(cleaned_quote == "Hello World");
    std::cout << "✅ clean_model_quote test passed!" << std::endl;
    
    // Test is_valid_email
    assert(TextProcessing::is_valid_email("test@example.com") == true);
    assert(TextProcessing::is_valid_email("invalid-email") == false);
    std::cout << "✅ is_valid_email test passed!" << std::endl;
    
    // Test count_punctuation
    std::string text_with_punctuation = "Hello, World! How are you?";
    int punct_count = TextProcessing::count_punctuation(text_with_punctuation);
    assert(punct_count == 3); // comma, exclamation, question mark
    (void)punct_count; // Suppress unused variable warning
    std::cout << "✅ count_punctuation test passed!" << std::endl;
    
    // Test decode_escapes
    std::string escaped_text = "Hello\\nWorld\\tTest";
    std::string decoded = TextProcessing::decode_escapes(escaped_text);
    assert(decoded.find("\n") != std::string::npos);
    assert(decoded.find("\t") != std::string::npos);
    std::cout << "✅ decode_escapes test passed!" << std::endl;
    
    // Test extract_embedded_json
    std::string text_with_json = "Some text {\"key\": \"value\"} more text";
    std::string extracted_json = TextProcessing::extract_embedded_json(text_with_json);
    assert(extracted_json == "{\"key\": \"value\"}");
    std::cout << "✅ extract_embedded_json test passed!" << std::endl;
    
    // Test constants
    assert(std::string(TextProcessing::RETURN_SEPARATOR) == "\n\r\n");
    assert(std::string(TextProcessing::SECTION_SEPARATOR) == "\n\n");
    assert(std::string(TextProcessing::INDEX_SEPARATOR) == "===");
    std::cout << "✅ constants test passed!" << std::endl;
}

void test_simple_tokenizer() {
    std::cout << "Testing SimpleTokenizer..." << std::endl;
    
    auto tokenizer = std::make_shared<SimpleTokenizer>();
    
    std::string text = "Hello world! This is a test.";
    auto tokens = tokenizer->tokenize(text);
    auto encoded = tokenizer->encode(text);
    auto decoded = tokenizer->decode(encoded);
    int count = tokenizer->count_tokens(text);
    
    assert(!tokens.empty());
    assert(!encoded.empty());
    assert(count > 0);
    assert(count == static_cast<int>(tokens.size()));
    (void)count; // Suppress unused variable warning
    
    std::cout << "✅ SimpleTokenizer test passed!" << std::endl;
}

void test_sentence_tokenizer() {
    std::cout << "Testing SentenceTokenizer..." << std::endl;
    
    auto tokenizer = std::make_shared<SentenceTokenizer>(true);
    
    std::string text = "Hello world! This is a test. How are you? I'm doing well.";
    auto tokens = tokenizer->tokenize(text);
    auto encoded = tokenizer->encode(text);
    auto decoded = tokenizer->decode(encoded);
    int count = tokenizer->count_tokens(text);
    
    assert(!tokens.empty());
    assert(!encoded.empty());
    assert(count > 0);
    assert(count == static_cast<int>(tokens.size()));
    (void)count; // Suppress unused variable warning
    
    // Check for sentence boundary tokens
    bool has_punctuation_tokens = false;
    for (const auto& token : tokens) {
        if (token == "." || token == "!" || token == "?") {
            has_punctuation_tokens = true;
            break;
        }
    }
    assert(has_punctuation_tokens);
    (void)has_punctuation_tokens; // Suppress unused variable warning
    
    std::cout << "✅ SentenceTokenizer test passed!" << std::endl;
}

void test_bpe_tokenizer() {
    std::cout << "Testing BPETokenizer..." << std::endl;
    
    auto tokenizer = std::make_shared<BPETokenizer>(1000);
    
    // Train on a small corpus
    std::vector<std::string> corpus = {
        "Hello world this is a test",
        "The quick brown fox jumps over the lazy dog",
        "Machine learning is fascinating",
        "Natural language processing is complex"
    };
    
    tokenizer->train(corpus);
    
    std::string text = "Hello world test";
    auto tokens = tokenizer->tokenize(text);
    auto encoded = tokenizer->encode(text);
    auto decoded = tokenizer->decode(encoded);
    int count = tokenizer->count_tokens(text);
    
    assert(!tokens.empty());
    assert(!encoded.empty());
    assert(count > 0);
    assert(count == static_cast<int>(tokens.size()));
    (void)count; // Suppress unused variable warning
    
    // Test save/load vocabulary
    tokenizer->save_vocabulary("test_vocab.txt");
    
    auto new_tokenizer = std::make_shared<BPETokenizer>();
    new_tokenizer->load_vocabulary("test_vocab.txt");
    
    auto new_tokens = new_tokenizer->tokenize(text);
    // The new tokenizer should produce some tokens (may not be identical due to vocabulary differences)
    assert(!new_tokens.empty());
    
    std::cout << "✅ BPETokenizer test passed!" << std::endl;
}

void test_tokenizer_factory() {
    std::cout << "Testing TokenizerFactory..." << std::endl;
    
    // Test factory creation
    auto simple_tokenizer = TokenizerFactory::create(TokenizerFactory::Type::SIMPLE);
    auto sentence_tokenizer = TokenizerFactory::create(TokenizerFactory::Type::SENTENCE);
    auto bpe_tokenizer = TokenizerFactory::create(TokenizerFactory::Type::BPE);
    
    assert(simple_tokenizer != nullptr);
    assert(sentence_tokenizer != nullptr);
    assert(bpe_tokenizer != nullptr);
    
    // Test config-based creation
    auto config_simple = TokenizerFactory::create_from_config("simple");
    auto config_sentence = TokenizerFactory::create_from_config("sentence");
    auto config_bpe = TokenizerFactory::create_from_config("bpe");
    
    assert(config_simple != nullptr);
    assert(config_sentence != nullptr);
    assert(config_bpe != nullptr);
    
    std::cout << "✅ TokenizerFactory test passed!" << std::endl;
}

void test_advanced_tokenizer_integration() {
    std::cout << "Testing advanced tokenizer integration..." << std::endl;
    
    // Test different tokenizers on the same text
    std::string test_text = "Hello world! This is a test sentence. How are you?";
    
    auto simple_tokenizer = TokenizerFactory::create(TokenizerFactory::Type::SIMPLE);
    auto sentence_tokenizer = TokenizerFactory::create(TokenizerFactory::Type::SENTENCE);
    auto bpe_tokenizer = TokenizerFactory::create(TokenizerFactory::Type::BPE);
    
    auto simple_tokens = simple_tokenizer->tokenize(test_text);
    auto sentence_tokens = sentence_tokenizer->tokenize(test_text);
    auto bpe_tokens = bpe_tokenizer->tokenize(test_text);
    
    // All should produce tokens
    assert(!simple_tokens.empty());
    assert(!sentence_tokens.empty());
    assert(!bpe_tokens.empty());
    
    // Token counts should be reasonable
    assert(simple_tokenizer->count_tokens(test_text) > 0);
    assert(sentence_tokenizer->count_tokens(test_text) > 0);
    assert(bpe_tokenizer->count_tokens(test_text) > 0);
    
    std::cout << "✅ Advanced tokenizer integration test passed!" << std::endl;
}

int main() {
    std::cout << "🧪 Running Advanced Features Tests" << std::endl;
    std::cout << "==================================" << std::endl;

    try {
        test_text_processing_utilities();
        test_simple_tokenizer();
        test_sentence_tokenizer();
        test_bpe_tokenizer();
        test_tokenizer_factory();
        test_advanced_tokenizer_integration();

        std::cout << "\n🎉 All advanced features tests passed successfully!" << std::endl;
        std::cout << "✅ R3M now has sophisticated text processing and tokenization!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
} 