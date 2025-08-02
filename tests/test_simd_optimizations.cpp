#include <iostream>
#include <chrono>
#include <random>
#include <string>
#include <cassert>
#include "r3m/utils/simd_utils.hpp"
#include "r3m/utils/text_processing.hpp"

void test_simd_capabilities() {
    std::cout << "=== SIMD Capability Detection ===" << std::endl;
    std::cout << "SIMD supported: " << (r3m::utils::SIMDUtils::supports_simd() ? "YES" : "NO") << std::endl;
    std::cout << "AVX2 supported: " << (r3m::utils::SIMDUtils::supports_avx2() ? "YES" : "NO") << std::endl;
    std::cout << "AVX-512 supported: " << (r3m::utils::SIMDUtils::supports_avx512() ? "YES" : "NO") << std::endl;
    std::cout << std::endl;
}

void test_character_counting() {
    std::cout << "=== Character Counting Tests ===" << std::endl;
    
    std::string test_text = "Hello world! This is a test with multiple spaces.";
    
    // Test basic character counting
    size_t space_count_simd = r3m::utils::SIMDUtils::count_char_simd(test_text, ' ');
    size_t space_count_scalar = r3m::utils::SIMDUtils::count_char_scalar(test_text, ' ');
    
    assert(space_count_simd == space_count_scalar);
    std::cout << "Space count: " << space_count_simd << std::endl;
    (void)space_count_scalar; // Suppress unused variable warning
    
    // Performance test
    std::string large_text;
    large_text.reserve(100000);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> char_dist(32, 126);
    
    for (int i = 0; i < 100000; ++i) {
        large_text += static_cast<char>(char_dist(gen));
        if (i % 100 == 0) large_text += ' ';
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    size_t simd_result = r3m::utils::SIMDUtils::count_char_simd(large_text, ' ');
    auto simd_end = std::chrono::high_resolution_clock::now();
    auto simd_time = std::chrono::duration_cast<std::chrono::microseconds>(simd_end - start);
    
    start = std::chrono::high_resolution_clock::now();
    size_t scalar_result = r3m::utils::SIMDUtils::count_char_scalar(large_text, ' ');
    auto scalar_end = std::chrono::high_resolution_clock::now();
    auto scalar_time = std::chrono::duration_cast<std::chrono::microseconds>(scalar_end - start);
    
    assert(simd_result == scalar_result);
    (void)simd_result; // Suppress unused variable warning
    (void)scalar_result; // Suppress unused variable warning
    std::cout << "SIMD time: " << simd_time.count() << " microseconds" << std::endl;
    std::cout << "Scalar time: " << scalar_time.count() << " microseconds" << std::endl;
    
    if (r3m::utils::SIMDUtils::supports_simd()) {
        double speedup = static_cast<double>(scalar_time.count()) / simd_time.count();
        std::cout << "SIMD speedup: " << speedup << "x" << std::endl;
    }
    std::cout << std::endl;
}

void test_whitespace_counting() {
    std::cout << "=== Whitespace Counting Tests ===" << std::endl;
    
    std::string test_text = "Hello\tworld!\nThis is a test\r\nwith multiple whitespace.";
    
    size_t whitespace_count_simd = r3m::utils::SIMDUtils::count_whitespace_simd(test_text);
    size_t whitespace_count_scalar = r3m::utils::SIMDUtils::count_whitespace_scalar(test_text);
    
    assert(whitespace_count_simd == whitespace_count_scalar);
    std::cout << "Whitespace count: " << whitespace_count_simd << std::endl;
    (void)whitespace_count_scalar; // Suppress unused variable warning
    std::cout << std::endl;
}

void test_punctuation_counting() {
    std::cout << "=== Punctuation Counting Tests ===" << std::endl;
    
    std::string test_text = "Hello, world! How are you? This is a test; with lots of punctuation: dots, commas, and more.";
    
    size_t punct_count_simd = r3m::utils::SIMDUtils::count_punctuation_simd(test_text);
    size_t punct_count_scalar = r3m::utils::SIMDUtils::count_punctuation_scalar(test_text);
    
    assert(punct_count_simd == punct_count_scalar);
    std::cout << "Punctuation count: " << punct_count_simd << std::endl;
    (void)punct_count_scalar; // Suppress unused variable warning
    std::cout << std::endl;
}

void test_text_cleaning() {
    std::cout << "=== Text Cleaning Tests ===" << std::endl;
    
    std::vector<char> chars_to_remove = {'<', '>', '&', '@', '#', '$', '%', '^', '*', '(', ')'};
    std::string test_text = "Text with <special> characters & symbols @#$%^&*()";
    
    std::string cleaned_simd = r3m::utils::SIMDUtils::clean_text_simd(test_text, chars_to_remove);
    std::string cleaned_scalar = r3m::utils::SIMDUtils::clean_text_scalar(test_text, chars_to_remove);
    
    assert(cleaned_simd == cleaned_scalar);
    std::cout << "Original: " << test_text << std::endl;
    std::cout << "Cleaned: " << cleaned_simd << std::endl;
    (void)cleaned_scalar; // Suppress unused variable warning
    std::cout << std::endl;
}

void test_token_counting() {
    std::cout << "=== Token Counting Tests ===" << std::endl;
    
    std::string test_text = "Hello world! This is a test with multiple tokens.";
    
    size_t token_count_simd = r3m::utils::SIMDUtils::count_tokens_simd(test_text);
    size_t token_count_scalar = r3m::utils::SIMDUtils::count_tokens_scalar(test_text);
    
    assert(token_count_simd == token_count_scalar);
    std::cout << "Token count: " << token_count_simd << std::endl;
    (void)token_count_scalar; // Suppress unused variable warning
    std::cout << std::endl;
}

void test_string_splitting() {
    std::cout << "=== String Splitting Tests ===" << std::endl;
    
    std::string test_text = "Hello world! This is a test with multiple words.";
    
    auto tokens_simd = r3m::utils::SIMDUtils::split_by_delimiter_simd(test_text, ' ');
    auto tokens_scalar = r3m::utils::SIMDUtils::split_by_delimiter_scalar(test_text, ' ');
    
    assert(tokens_simd.size() == tokens_scalar.size());
    for (size_t i = 0; i < tokens_simd.size(); ++i) {
        assert(tokens_simd[i] == tokens_scalar[i]);
    }
    
    std::cout << "Split into " << tokens_simd.size() << " tokens" << std::endl;
    for (const auto& token : tokens_simd) {
        std::cout << "  - '" << token << "'" << std::endl;
    }
    std::cout << std::endl;
}

void test_text_processing_integration() {
    std::cout << "=== Text Processing Integration Tests ===" << std::endl;
    
    std::string test_text = "Hello world! This is a test with multiple sentences. How are you? I'm doing well.";
    
    // Test integration with existing text processing
    std::string cleaned = r3m::utils::TextProcessing::clean_text(test_text);
    size_t words = r3m::utils::TextProcessing::count_words(test_text);
    size_t sentences = r3m::utils::TextProcessing::count_sentences(test_text);
    size_t paragraphs = r3m::utils::TextProcessing::count_paragraphs(test_text);
    
    std::cout << "Original text: " << test_text << std::endl;
    std::cout << "Words: " << words << std::endl;
    std::cout << "Sentences: " << sentences << std::endl;
    std::cout << "Paragraphs: " << paragraphs << std::endl;
    std::cout << std::endl;
}

void test_large_document_performance() {
    std::cout << "=== Large Document Performance Test ===" << std::endl;
    
    // Generate a large document
    std::string large_text;
    large_text.reserve(1000000); // 1MB
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> char_dist(32, 126);
    
    for (int i = 0; i < 1000000; ++i) {
        large_text += static_cast<char>(char_dist(gen));
        if (i % 50 == 0) large_text += ' ';
        if (i % 200 == 0) large_text += ". ";
    }
    
    std::cout << "Generated " << large_text.length() << " character document" << std::endl;
    
    // Performance test for large document processing
    auto start = std::chrono::high_resolution_clock::now();
    
    // SIMD-optimized operations
    size_t words = r3m::utils::TextProcessing::count_words(large_text);
    size_t sentences = r3m::utils::TextProcessing::count_sentences(large_text);
    std::string cleaned = r3m::utils::TextProcessing::clean_text(large_text);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Large document processing completed in " << duration.count() << " ms" << std::endl;
    std::cout << "Words: " << words << ", Sentences: " << sentences << std::endl;
    std::cout << "Processing speed: " << (large_text.length() / duration.count()) << " chars/ms" << std::endl;
    std::cout << std::endl;
}

void test_bpe_token_matching() {
    std::cout << "=== BPE Token Matching Tests ===" << std::endl;
    
    // Generate test text with common character pairs
    std::string test_text = "hello world! this is a test with common pairs like th, he, ll, oo, wo, ld, is, te, st.";
    
    // Common BPE pairs to search for
    std::vector<std::string> pairs = {"th", "he", "ll", "oo", "wo", "ld", "is", "te", "st", "an", "er", "in", "on", "at"};
    
    // Test SIMD vs scalar BPE pair finding
    auto simd_positions = r3m::utils::SIMDUtils::find_bpe_pairs_simd(test_text, pairs);
    auto scalar_positions = r3m::utils::SIMDUtils::find_bpe_pairs_scalar(test_text, pairs);
    
    assert(simd_positions.size() == scalar_positions.size());
    std::cout << "Found " << simd_positions.size() << " BPE pairs" << std::endl;
    
    // Performance test with larger text
    std::string large_text;
    large_text.reserve(100000);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> char_dist(97, 122); // a-z
    
    for (int i = 0; i < 100000; ++i) {
        large_text += static_cast<char>(char_dist(gen));
        if (i % 100 == 0) large_text += " the and for with ";
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    auto simd_result = r3m::utils::SIMDUtils::find_bpe_pairs_simd(large_text, pairs);
    auto simd_end = std::chrono::high_resolution_clock::now();
    auto simd_time = std::chrono::duration_cast<std::chrono::microseconds>(simd_end - start);
    
    start = std::chrono::high_resolution_clock::now();
    auto scalar_result = r3m::utils::SIMDUtils::find_bpe_pairs_scalar(large_text, pairs);
    auto scalar_end = std::chrono::high_resolution_clock::now();
    auto scalar_time = std::chrono::duration_cast<std::chrono::microseconds>(scalar_end - start);
    
    assert(simd_result.size() == scalar_result.size());
    std::cout << "SIMD BPE time: " << simd_time.count() << " microseconds" << std::endl;
    std::cout << "Scalar BPE time: " << scalar_time.count() << " microseconds" << std::endl;
    
    if (r3m::utils::SIMDUtils::supports_simd()) {
        double speedup = static_cast<double>(scalar_time.count()) / simd_time.count();
        std::cout << "BPE SIMD speedup: " << speedup << "x" << std::endl;
    }
    std::cout << std::endl;
}

void test_sentence_boundary_detection() {
    std::cout << "=== Sentence Boundary Detection Tests ===" << std::endl;
    
    std::string test_text = "Hello world! How are you? This is a test. I'm doing well.\nThis is a new line.";
    
    auto simd_boundaries = r3m::utils::SIMDUtils::find_sentence_boundaries_simd(test_text);
    auto scalar_boundaries = r3m::utils::SIMDUtils::find_sentence_boundaries_scalar(test_text);
    
    assert(simd_boundaries.size() == scalar_boundaries.size());
    std::cout << "Found " << simd_boundaries.size() << " sentence boundaries" << std::endl;
    
    // Performance test with larger text
    std::string large_text;
    large_text.reserve(100000);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> char_dist(97, 122); // a-z
    
    for (int i = 0; i < 100000; ++i) {
        large_text += static_cast<char>(char_dist(gen));
        if (i % 50 == 0) large_text += ". ";
        if (i % 100 == 0) large_text += "! ";
        if (i % 150 == 0) large_text += "? ";
        if (i % 200 == 0) large_text += "\n";
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    auto simd_result = r3m::utils::SIMDUtils::find_sentence_boundaries_simd(large_text);
    auto simd_end = std::chrono::high_resolution_clock::now();
    auto simd_time = std::chrono::duration_cast<std::chrono::microseconds>(simd_end - start);
    
    start = std::chrono::high_resolution_clock::now();
    auto scalar_result = r3m::utils::SIMDUtils::find_sentence_boundaries_scalar(large_text);
    auto scalar_end = std::chrono::high_resolution_clock::now();
    auto scalar_time = std::chrono::duration_cast<std::chrono::microseconds>(scalar_end - start);
    
    assert(simd_result.size() == scalar_result.size());
    std::cout << "SIMD sentence detection time: " << simd_time.count() << " microseconds" << std::endl;
    std::cout << "Scalar sentence detection time: " << scalar_time.count() << " microseconds" << std::endl;
    
    if (r3m::utils::SIMDUtils::supports_simd()) {
        double speedup = static_cast<double>(scalar_time.count()) / simd_time.count();
        std::cout << "Sentence detection SIMD speedup: " << speedup << "x" << std::endl;
    }
    std::cout << std::endl;
}

void test_pattern_matching() {
    std::cout << "=== Pattern Matching Tests ===" << std::endl;
    
    std::string test_text = "hello world! this is a test with patterns like th, he, ll, oo.";
    std::string pattern = "th";
    
    auto simd_positions = r3m::utils::SIMDUtils::find_pattern_simd(test_text, pattern);
    auto scalar_positions = r3m::utils::SIMDUtils::find_pattern_scalar(test_text, pattern);
    
    assert(simd_positions.size() == scalar_positions.size());
    std::cout << "Found " << simd_positions.size() << " occurrences of pattern '" << pattern << "'" << std::endl;
    std::cout << std::endl;
}

void test_text_normalization() {
    std::cout << "=== Text Normalization Tests ===" << std::endl;
    
    std::string test_text = "Hello\tworld!\nThis is a test\r\nwith multiple whitespace.";
    
    auto simd_normalized = r3m::utils::SIMDUtils::normalize_for_search_simd(test_text);
    auto scalar_normalized = r3m::utils::SIMDUtils::normalize_for_search_scalar(test_text);
    
    assert(simd_normalized == scalar_normalized);
    std::cout << "Original: " << test_text << std::endl;
    std::cout << "Normalized: " << simd_normalized << std::endl;
    
    // Performance test with larger text
    std::string large_text;
    large_text.reserve(100000);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> char_dist(97, 122); // a-z
    
    for (int i = 0; i < 100000; ++i) {
        large_text += static_cast<char>(char_dist(gen));
        if (i % 50 == 0) large_text += " \t\n\r ";
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    auto simd_result = r3m::utils::SIMDUtils::normalize_for_search_simd(large_text);
    auto simd_end = std::chrono::high_resolution_clock::now();
    auto simd_time = std::chrono::duration_cast<std::chrono::microseconds>(simd_end - start);
    
    start = std::chrono::high_resolution_clock::now();
    auto scalar_result = r3m::utils::SIMDUtils::normalize_for_search_scalar(large_text);
    auto scalar_end = std::chrono::high_resolution_clock::now();
    auto scalar_time = std::chrono::duration_cast<std::chrono::microseconds>(scalar_end - start);
    
    assert(simd_result == scalar_result);
    std::cout << "SIMD normalization time: " << simd_time.count() << " microseconds" << std::endl;
    std::cout << "Scalar normalization time: " << scalar_time.count() << " microseconds" << std::endl;
    
    if (r3m::utils::SIMDUtils::supports_simd()) {
        double speedup = static_cast<double>(scalar_time.count()) / simd_time.count();
        std::cout << "Text normalization SIMD speedup: " << speedup << "x" << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "R3M SIMD Optimizations Test" << std::endl;
    std::cout << "============================" << std::endl;
    std::cout << std::endl;
    
    test_simd_capabilities();
    test_character_counting();
    test_whitespace_counting();
    test_punctuation_counting();
    test_text_cleaning();
    test_token_counting();
    test_string_splitting();
    test_text_processing_integration();
    test_large_document_performance();
    test_bpe_token_matching();
    test_sentence_boundary_detection();
    test_pattern_matching();
    test_text_normalization();
    
    std::cout << "All SIMD tests passed!" << std::endl;
    return 0;
} 