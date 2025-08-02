#pragma once

#include <string>
#include <vector>

// Cross-platform SIMD support
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #include <immintrin.h>  // x86 intrinsics
    #define R3M_SIMD_X86_AVAILABLE
#elif defined(__aarch64__) || defined(__arm64__)
    #include <arm_neon.h>   // ARM NEON intrinsics
    #define R3M_SIMD_ARM_AVAILABLE
#endif

namespace r3m::utils {

class SIMDUtils {
public:
    // SIMD-optimized character counting
    static size_t count_char_simd(const std::string& text, char target);
    
    // SIMD-optimized string search
    static size_t find_substring_simd(const std::string& text, const std::string& pattern);
    
    // SIMD-optimized whitespace counting
    static size_t count_whitespace_simd(const std::string& text);
    
    // SIMD-optimized punctuation counting
    static size_t count_punctuation_simd(const std::string& text);
    
    // SIMD-optimized text cleaning (remove specific characters)
    static std::string clean_text_simd(const std::string& text, const std::vector<char>& chars_to_remove);
    
    // SIMD-optimized token counting (approximate)
    static size_t count_tokens_simd(const std::string& text);
    
    // SIMD-optimized string splitting by delimiter
    static std::vector<std::string> split_by_delimiter_simd(const std::string& text, char delimiter);
    
    // BPE token matching - find character pairs in parallel
    static std::vector<size_t> find_bpe_pairs_simd(const std::string& text, const std::vector<std::string>& pairs);
    
    // Fast sentence boundary detection
    static std::vector<size_t> find_sentence_boundaries_simd(const std::string& text);
    
    // Multi-character pattern matching
    static std::vector<size_t> find_pattern_simd(const std::string& text, const std::string& pattern);
    
    // Text normalization for vector search
    static std::string normalize_for_search_simd(const std::string& text);
    
    // Check if CPU supports SIMD
    static bool supports_simd();
    
    // Check if CPU supports AVX2 (x86 only)
    static bool supports_avx2();
    
    // Check if CPU supports AVX-512 (x86 only)
    static bool supports_avx512();
    
    // Scalar fallback implementations for testing
    static size_t count_char_scalar(const std::string& text, char target);
    static size_t find_substring_scalar(const std::string& text, const std::string& pattern);
    static size_t count_whitespace_scalar(const std::string& text);
    static size_t count_punctuation_scalar(const std::string& text);
    static std::string clean_text_scalar(const std::string& text, const std::vector<char>& chars_to_remove);
    static size_t count_tokens_scalar(const std::string& text);
    static std::vector<std::string> split_by_delimiter_scalar(const std::string& text, char delimiter);
    
    // BPE and advanced text processing scalar fallbacks
    static std::vector<size_t> find_bpe_pairs_scalar(const std::string& text, const std::vector<std::string>& pairs);
    static std::vector<size_t> find_sentence_boundaries_scalar(const std::string& text);
    static std::vector<size_t> find_pattern_scalar(const std::string& text, const std::string& pattern);
    static std::string normalize_for_search_scalar(const std::string& text);

private:
    // Helper function to verify CPU capabilities
    static void check_cpu_support();
};

} // namespace r3m::utils 