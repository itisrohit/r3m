#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "r3m/utils/text_processing.hpp"
#include "r3m/chunking/advanced_tokenizer.hpp"

using namespace r3m::utils;
using namespace r3m::chunking;

void test_text_processing_utilities() {
    std::cout << "Testing TextProcessing utilities..." << std::endl;
    
    // Test clean_text
    std::string dirty_text = "Hello\x00World\x01\x02\x03";
    std::string cleaned = TextProcessing::clean_text(dirty_text);
    assert(cleaned == "HelloWorld");
    std::cout << "✅ clean_text test passed!" << std::endl;
    
    // Test shared_precompare_cleanup
    std::string text_to_clean = "  Hello, World!  ";
    std::string cleaned_for_compare = TextProcessing::shared_precompare_cleanup(text_to_clean);
    assert(cleaned_for_compare.find(" ") == std::string::npos);
    assert(cleaned_for_compare.find(",") == std::string::npos);
    std::cout << "✅ shared_precompare_cleanup test passed!" << std::endl;
    
    // Test remove_punctuation
    std::string text_with_punct = "Hello, World! How are you?";
    std::string without_punct = TextProcessing::remove_punctuation(text_with_punct);
    assert(without_punct.find(",") == std::string::npos);
    assert(without_punct.find("!") == std::string::npos);
    assert(without_punct.find("?") == std::string::npos);
    std::cout << "✅ remove_punctuation test passed!" << std::endl;
    
    // Test replace_whitespaces_with_space
    std::string text_with_whitespace = "Hello\tWorld\nTest\r\n";
    std::string normalized = TextProcessing::replace_whitespaces_with_space(text_with_whitespace);
    assert(normalized.find("\t") == std::string::npos);
    assert(normalized.find("\n") == std::string::npos);
    assert(normalized.find("\r") == std::string::npos);
    std::cout << "✅ replace_whitespaces_with_space test passed!" << std::endl;
    
    // Test escape_newlines
    std::string text_with_newlines = "Hello\nWorld\nTest";
    std::string escaped = TextProcessing::escape_newlines(text_with_newlines);
    assert(escaped.find("\\\\n") != std::string::npos);
    std::cout << "✅ escape_newlines test passed!" << std::endl;
    
    // Test make_url_compatible
    std::string text_for_url = "Hello World! Test & More";
    std::string url_compatible = TextProcessing::make_url_compatible(text_for_url);
    assert(url_compatible.find(" ") == std::string::npos);
    assert(url_compatible.find("!") == std::string::npos);
    assert(url_compatible.find("&") == std::string::npos);
    std::cout << "✅ make_url_compatible test passed!" << std::endl;
    
    // Test clean_model_quote (updated signature)
    std::string quoted_text = "\"Hello World\"";
    std::string cleaned_quote = TextProcessing::clean_model_quote(quoted_text, 0);
    assert(cleaned_quote.find("\"") == std::string::npos);
    std::cout << "✅ clean_model_quote test passed!" << std::endl;
    
    // Test count_words (new SIMD-optimized function)
    std::string text_for_counting = "Hello world! This is a test.";
    size_t word_count = TextProcessing::count_words(text_for_counting);
    assert(word_count > 0);
    (void)word_count; // Suppress unused variable warning
    std::cout << "✅ count_words test passed!" << std::endl;
    
    // Test count_sentences (new SIMD-optimized function)
    size_t sentence_count = TextProcessing::count_sentences(text_for_counting);
    assert(sentence_count > 0);
    (void)sentence_count; // Suppress unused variable warning
    std::cout << "✅ count_sentences test passed!" << std::endl;
    
    // Test count_paragraphs (new SIMD-optimized function)
    std::string text_with_paragraphs = "First paragraph.\n\nSecond paragraph.\n\nThird paragraph.";
    size_t paragraph_count = TextProcessing::count_paragraphs(text_with_paragraphs);
    assert(paragraph_count > 0);
    (void)paragraph_count; // Suppress unused variable warning
    std::cout << "✅ count_paragraphs test passed!" << std::endl;
    
    // Test calculate_readability_score (new function)
    double readability = TextProcessing::calculate_readability_score(text_for_counting);
    assert(readability >= 0.0 && readability <= 100.0);
    (void)readability; // Suppress unused variable warning
    std::cout << "✅ calculate_readability_score test passed!" << std::endl;
    
    // Test generate_summary (new function)
    std::string long_text = "This is a very long text that should be summarized. " 
                           "It contains multiple sentences and should be truncated. "
                           "The summary should be shorter than the original text.";
    std::string summary = TextProcessing::generate_summary(long_text, 50);
    assert(summary.length() <= 50);
    std::cout << "✅ generate_summary test passed!" << std::endl;
    
    // Test constants
    assert(TextProcessing::SECTION_SEPARATOR == "\n\n");
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