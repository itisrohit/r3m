#include "r3m/utils/simd_utils.hpp"
#include <cstring>
#include <algorithm>
#include <cctype>
#include <sstream>

namespace r3m::utils {

// CPU capability detection
static bool simd_supported = false;
static bool avx2_supported = false;
static bool avx512_supported = false;
static bool capabilities_checked = false;

void SIMDUtils::check_cpu_support() {
    if (capabilities_checked) return;
    
    #ifdef R3M_SIMD_X86_AVAILABLE
        // Check for AVX2 support on x86
        #ifdef __AVX2__
            avx2_supported = true;
        #else
            avx2_supported = false;
        #endif
        
        // Check for AVX-512 support on x86
        #ifdef __AVX512F__
            avx512_supported = true;
        #else
            avx512_supported = false;
        #endif
        
        simd_supported = avx2_supported || avx512_supported;
    #elif defined(R3M_SIMD_ARM_AVAILABLE)
        // ARM NEON is always available on ARM64
        simd_supported = true;
        avx2_supported = false;  // Not applicable on ARM
        avx512_supported = false; // Not applicable on ARM
    #else
        simd_supported = false;
        avx2_supported = false;
        avx512_supported = false;
    #endif
    
    capabilities_checked = true;
}

bool SIMDUtils::supports_simd() {
    check_cpu_support();
    return simd_supported;
}

bool SIMDUtils::supports_avx2() {
    check_cpu_support();
    return avx2_supported;
}

bool SIMDUtils::supports_avx512() {
    check_cpu_support();
    return avx512_supported;
}

// Cross-platform SIMD-optimized character counting
size_t SIMDUtils::count_char_simd(const std::string& text, char target) {
    if (!supports_simd()) {
        return count_char_scalar(text, target);
    }
    
    size_t count = 0;
    const char* ptr = text.data();
    size_t len = text.length();
    
    #ifdef R3M_SIMD_X86_AVAILABLE
        if (supports_avx2()) {
            // Load target character into 256-bit vector
            __m256i target_vec = _mm256_set1_epi8(target);
            
            size_t i = 0;
            // Process 32 characters at a time with AVX2
            for (; i + 32 <= len; i += 32) {
                __m256i chunk = _mm256_loadu_si256((__m256i*)(ptr + i));
                __m256i cmp = _mm256_cmpeq_epi8(chunk, target_vec);
                int mask = _mm256_movemask_epi8(cmp);
                count += __builtin_popcount(mask);
            }
            
            // Scalar fallback for remaining characters
            for (; i < len; ++i) {
                if (ptr[i] == target) count++;
            }
        } else {
            return count_char_scalar(text, target);
        }
    #elif defined(R3M_SIMD_ARM_AVAILABLE)
        // ARM NEON implementation
        uint8x16_t target_vec = vdupq_n_u8(target);
        
        size_t i = 0;
        // Process 16 characters at a time with NEON
        for (; i + 16 <= len; i += 16) {
            uint8x16_t chunk = vld1q_u8((uint8_t*)(ptr + i));
            uint8x16_t cmp = vceqq_u8(chunk, target_vec);
            uint64x2_t mask = vreinterpretq_u64_u8(cmp);
            uint64_t mask_low = vgetq_lane_u64(mask, 0);
            uint64_t mask_high = vgetq_lane_u64(mask, 1);
            count += __builtin_popcountll(mask_low) + __builtin_popcountll(mask_high);
        }
        
        // Scalar fallback for remaining characters
        for (; i < len; ++i) {
            if (ptr[i] == target) count++;
        }
    #else
        return count_char_scalar(text, target);
    #endif
    
    return count;
}

// Cross-platform SIMD-optimized string search
size_t SIMDUtils::find_substring_simd(const std::string& text, const std::string& pattern) {
    if (!supports_simd() || pattern.empty()) {
        return find_substring_scalar(text, pattern);
    }
    
    const char* text_ptr = text.data();
    const char* pattern_ptr = pattern.data();
    size_t text_len = text.length();
    size_t pattern_len = pattern.length();
    
    if (pattern_len > text_len) {
        return std::string::npos;
    }
    
    #ifdef R3M_SIMD_X86_AVAILABLE
        if (supports_avx2()) {
            // Load first character of pattern into 256-bit vector
            __m256i pattern_first = _mm256_set1_epi8(pattern_ptr[0]);
            
            for (size_t i = 0; i <= text_len - 32; i += 32) {
                // Load 32 characters from text
                __m256i text_chunk = _mm256_loadu_si256((__m256i*)(text_ptr + i));
                
                // Compare with first character of pattern
                __m256i cmp = _mm256_cmpeq_epi8(text_chunk, pattern_first);
                int mask = _mm256_movemask_epi8(cmp);
                
                if (mask != 0) {
                    // Found potential matches, verify each one
                    for (int j = 0; j < 32; j++) {
                        if (mask & (1 << j)) {
                            size_t pos = i + j;
                            if (pos + pattern_len <= text_len) {
                                if (memcmp(text_ptr + pos, pattern_ptr, pattern_len) == 0) {
                                    return pos;
                                }
                            }
                        }
                    }
                }
            }
            
            // Scalar fallback for remaining text
            for (size_t i = (text_len / 32) * 32; i <= text_len - pattern_len; ++i) {
                if (memcmp(text_ptr + i, pattern_ptr, pattern_len) == 0) {
                    return i;
                }
            }
        } else {
            return find_substring_scalar(text, pattern);
        }
    #elif defined(R3M_SIMD_ARM_AVAILABLE)
        // ARM NEON implementation
        uint8x16_t pattern_first = vdupq_n_u8(pattern_ptr[0]);
        
        for (size_t i = 0; i <= text_len - 16; i += 16) {
            uint8x16_t text_chunk = vld1q_u8((uint8_t*)(text_ptr + i));
            uint8x16_t cmp = vceqq_u8(text_chunk, pattern_first);
            uint64x2_t mask = vreinterpretq_u64_u8(cmp);
            uint64_t mask_low = vgetq_lane_u64(mask, 0);
            uint64_t mask_high = vgetq_lane_u64(mask, 1);
            
            // Check for matches in the 16-character chunk
            for (int j = 0; j < 16; j++) {
                bool match = false;
                if (j < 8) {
                    match = (mask_low & (1ULL << j)) != 0;
                } else {
                    match = (mask_high & (1ULL << (j - 8))) != 0;
                }
                
                if (match) {
                    size_t pos = i + j;
                    if (pos + pattern_len <= text_len) {
                        if (memcmp(text_ptr + pos, pattern_ptr, pattern_len) == 0) {
                            return pos;
                        }
                    }
                }
            }
        }
        
        // Scalar fallback for remaining text
        for (size_t i = (text_len / 16) * 16; i <= text_len - pattern_len; ++i) {
            if (memcmp(text_ptr + i, pattern_ptr, pattern_len) == 0) {
                return i;
            }
        }
    #else
        return find_substring_scalar(text, pattern);
    #endif
    
    return std::string::npos;
}

// Cross-platform SIMD-optimized whitespace counting
size_t SIMDUtils::count_whitespace_simd(const std::string& text) {
    if (!supports_simd()) {
        return count_whitespace_scalar(text);
    }
    
    size_t count = 0;
    const char* ptr = text.data();
    size_t len = text.length();
    
    #ifdef R3M_SIMD_X86_AVAILABLE
        if (supports_avx2()) {
            // Create masks for different whitespace characters
            __m256i space_mask = _mm256_set1_epi8(' ');
            __m256i tab_mask = _mm256_set1_epi8('\t');
            __m256i newline_mask = _mm256_set1_epi8('\n');
            __m256i carriage_return_mask = _mm256_set1_epi8('\r');
            
            size_t i = 0;
            for (; i + 32 <= len; i += 32) {
                __m256i chunk = _mm256_loadu_si256((__m256i*)(ptr + i));
                
                // Check for each type of whitespace
                __m256i space_cmp = _mm256_cmpeq_epi8(chunk, space_mask);
                __m256i tab_cmp = _mm256_cmpeq_epi8(chunk, tab_mask);
                __m256i newline_cmp = _mm256_cmpeq_epi8(chunk, newline_mask);
                __m256i cr_cmp = _mm256_cmpeq_epi8(chunk, carriage_return_mask);
                
                // Combine all whitespace matches
                __m256i combined = _mm256_or_si256(_mm256_or_si256(space_cmp, tab_cmp),
                                                  _mm256_or_si256(newline_cmp, cr_cmp));
                
                int mask = _mm256_movemask_epi8(combined);
                count += __builtin_popcount(mask);
            }
            
            // Scalar fallback for remaining characters
            for (; i < len; ++i) {
                if (std::isspace(ptr[i])) count++;
            }
        } else {
            return count_whitespace_scalar(text);
        }
    #elif defined(R3M_SIMD_ARM_AVAILABLE)
        // ARM NEON implementation
        uint8x16_t space_mask = vdupq_n_u8(' ');
        uint8x16_t tab_mask = vdupq_n_u8('\t');
        uint8x16_t newline_mask = vdupq_n_u8('\n');
        uint8x16_t cr_mask = vdupq_n_u8('\r');
        
        size_t i = 0;
        for (; i + 16 <= len; i += 16) {
            uint8x16_t chunk = vld1q_u8((uint8_t*)(ptr + i));
            
            // Check for each type of whitespace
            uint8x16_t space_cmp = vceqq_u8(chunk, space_mask);
            uint8x16_t tab_cmp = vceqq_u8(chunk, tab_mask);
            uint8x16_t newline_cmp = vceqq_u8(chunk, newline_mask);
            uint8x16_t cr_cmp = vceqq_u8(chunk, cr_mask);
            
            // Combine all whitespace matches
            uint8x16_t combined = vorrq_u8(vorrq_u8(space_cmp, tab_cmp),
                                          vorrq_u8(newline_cmp, cr_cmp));
            
            uint64x2_t mask = vreinterpretq_u64_u8(combined);
            uint64_t mask_low = vgetq_lane_u64(mask, 0);
            uint64_t mask_high = vgetq_lane_u64(mask, 1);
            count += __builtin_popcountll(mask_low) + __builtin_popcountll(mask_high);
        }
        
        // Scalar fallback for remaining characters
        for (; i < len; ++i) {
            if (std::isspace(ptr[i])) count++;
        }
    #else
        return count_whitespace_scalar(text);
    #endif
    
    return count;
}

// Cross-platform SIMD-optimized punctuation counting
size_t SIMDUtils::count_punctuation_simd(const std::string& text) {
    if (!supports_simd()) {
        return count_punctuation_scalar(text);
    }
    
    size_t count = 0;
    const char* ptr = text.data();
    size_t len = text.length();
    
    #ifdef R3M_SIMD_X86_AVAILABLE
        if (supports_avx2()) {
            // Create masks for common punctuation characters
            __m256i period_mask = _mm256_set1_epi8('.');
            __m256i comma_mask = _mm256_set1_epi8(',');
            __m256i exclamation_mask = _mm256_set1_epi8('!');
            __m256i question_mask = _mm256_set1_epi8('?');
            __m256i semicolon_mask = _mm256_set1_epi8(';');
            __m256i colon_mask = _mm256_set1_epi8(':');
            
            size_t i = 0;
            for (; i + 32 <= len; i += 32) {
                __m256i chunk = _mm256_loadu_si256((__m256i*)(ptr + i));
                
                // Check for each type of punctuation
                __m256i period_cmp = _mm256_cmpeq_epi8(chunk, period_mask);
                __m256i comma_cmp = _mm256_cmpeq_epi8(chunk, comma_mask);
                __m256i exclamation_cmp = _mm256_cmpeq_epi8(chunk, exclamation_mask);
                __m256i question_cmp = _mm256_cmpeq_epi8(chunk, question_mask);
                __m256i semicolon_cmp = _mm256_cmpeq_epi8(chunk, semicolon_mask);
                __m256i colon_cmp = _mm256_cmpeq_epi8(chunk, colon_mask);
                
                // Combine all punctuation matches
                __m256i combined = _mm256_or_si256(_mm256_or_si256(period_cmp, comma_cmp),
                                                  _mm256_or_si256(exclamation_cmp, question_cmp));
                combined = _mm256_or_si256(combined, _mm256_or_si256(semicolon_cmp, colon_cmp));
                
                int mask = _mm256_movemask_epi8(combined);
                count += __builtin_popcount(mask);
            }
            
            // Scalar fallback for remaining characters
            for (; i < len; ++i) {
                if (std::ispunct(ptr[i])) count++;
            }
        } else {
            return count_punctuation_scalar(text);
        }
    #elif defined(R3M_SIMD_ARM_AVAILABLE)
        // ARM NEON implementation
        uint8x16_t period_mask = vdupq_n_u8('.');
        uint8x16_t comma_mask = vdupq_n_u8(',');
        uint8x16_t exclamation_mask = vdupq_n_u8('!');
        uint8x16_t question_mask = vdupq_n_u8('?');
        uint8x16_t semicolon_mask = vdupq_n_u8(';');
        uint8x16_t colon_mask = vdupq_n_u8(':');
        
        size_t i = 0;
        for (; i + 16 <= len; i += 16) {
            uint8x16_t chunk = vld1q_u8((uint8_t*)(ptr + i));
            
            // Check for each type of punctuation
            uint8x16_t period_cmp = vceqq_u8(chunk, period_mask);
            uint8x16_t comma_cmp = vceqq_u8(chunk, comma_mask);
            uint8x16_t exclamation_cmp = vceqq_u8(chunk, exclamation_mask);
            uint8x16_t question_cmp = vceqq_u8(chunk, question_mask);
            uint8x16_t semicolon_cmp = vceqq_u8(chunk, semicolon_mask);
            uint8x16_t colon_cmp = vceqq_u8(chunk, colon_mask);
            
            // Combine all punctuation matches
            uint8x16_t combined = vorrq_u8(vorrq_u8(period_cmp, comma_cmp),
                                          vorrq_u8(exclamation_cmp, question_cmp));
            combined = vorrq_u8(combined, vorrq_u8(semicolon_cmp, colon_cmp));
            
            uint64x2_t mask = vreinterpretq_u64_u8(combined);
            uint64_t mask_low = vgetq_lane_u64(mask, 0);
            uint64_t mask_high = vgetq_lane_u64(mask, 1);
            count += __builtin_popcountll(mask_low) + __builtin_popcountll(mask_high);
        }
        
        // Scalar fallback for remaining characters
        for (; i < len; ++i) {
            if (std::ispunct(ptr[i])) count++;
        }
    #else
        return count_punctuation_scalar(text);
    #endif
    
    return count;
}

// Cross-platform SIMD-optimized text cleaning
std::string SIMDUtils::clean_text_simd(const std::string& text, const std::vector<char>& chars_to_remove) {
    if (!supports_simd() || chars_to_remove.empty()) {
        return clean_text_scalar(text, chars_to_remove);
    }
    
    std::string result;
    result.reserve(text.length());
    
    const char* ptr = text.data();
    size_t len = text.length();
    
    // Create lookup table for characters to remove
    bool remove_chars[256] = {false};
    for (char c : chars_to_remove) {
        remove_chars[static_cast<unsigned char>(c)] = true;
    }
    
    size_t i = 0;
    for (; i < len; ++i) {
        if (!remove_chars[static_cast<unsigned char>(ptr[i])]) {
            result += ptr[i];
        }
    }
    
    return result;
}

// Cross-platform SIMD-optimized token counting (approximate)
size_t SIMDUtils::count_tokens_simd(const std::string& text) {
    if (!supports_simd()) {
        return count_tokens_scalar(text);
    }
    
    // Count spaces and add 1 for the first token
    size_t space_count = count_whitespace_simd(text);
    return space_count + 1;
}

// Cross-platform SIMD-optimized string splitting
std::vector<std::string> SIMDUtils::split_by_delimiter_simd(const std::string& text, char delimiter) {
    if (!supports_simd()) {
        return split_by_delimiter_scalar(text, delimiter);
    }
    
    std::vector<std::string> tokens;
    const char* ptr = text.data();
    size_t len = text.length();
    
    size_t start = 0;
    size_t i = 0;
    
    #ifdef R3M_SIMD_X86_AVAILABLE
        if (supports_avx2()) {
            // Use SIMD to find delimiters
            __m256i delim_vec = _mm256_set1_epi8(delimiter);
            
            for (; i + 32 <= len; i += 32) {
                __m256i chunk = _mm256_loadu_si256((__m256i*)(ptr + i));
                __m256i cmp = _mm256_cmpeq_epi8(chunk, delim_vec);
                int mask = _mm256_movemask_epi8(cmp);
                
                if (mask != 0) {
                    for (int j = 0; j < 32; j++) {
                        if (mask & (1 << j)) {
                            size_t pos = i + j;
                            if (pos > start) {
                                tokens.emplace_back(ptr + start, pos - start);
                            }
                            start = pos + 1;
                        }
                    }
                }
            }
        } else {
            return split_by_delimiter_scalar(text, delimiter);
        }
    #elif defined(R3M_SIMD_ARM_AVAILABLE)
        // ARM NEON implementation
        uint8x16_t delim_vec = vdupq_n_u8(delimiter);
        
        for (; i + 16 <= len; i += 16) {
            uint8x16_t chunk = vld1q_u8((uint8_t*)(ptr + i));
            uint8x16_t cmp = vceqq_u8(chunk, delim_vec);
            uint64x2_t mask = vreinterpretq_u64_u8(cmp);
            uint64_t mask_low = vgetq_lane_u64(mask, 0);
            uint64_t mask_high = vgetq_lane_u64(mask, 1);
            
            // Check for delimiters in the 16-character chunk
            for (int j = 0; j < 16; j++) {
                bool is_delimiter = false;
                if (j < 8) {
                    is_delimiter = (mask_low & (1ULL << j)) != 0;
                } else {
                    is_delimiter = (mask_high & (1ULL << (j - 8))) != 0;
                }
                
                if (is_delimiter) {
                    size_t pos = i + j;
                    if (pos > start) {
                        tokens.emplace_back(ptr + start, pos - start);
                    }
                    start = pos + 1;
                }
            }
        }
    #else
        return split_by_delimiter_scalar(text, delimiter);
    #endif
    
    // Scalar fallback for remaining characters
    for (; i < len; ++i) {
        if (ptr[i] == delimiter) {
            if (i > start) {
                tokens.emplace_back(ptr + start, i - start);
            }
            start = i + 1;
        }
    }
    
    // Add the last token if there's remaining text
    if (start < len) {
        tokens.emplace_back(ptr + start, len - start);
    }
    
    return tokens;
}

// BPE token matching - find character pairs in parallel
std::vector<size_t> SIMDUtils::find_bpe_pairs_simd(const std::string& text, const std::vector<std::string>& pairs) {
    if (!supports_simd()) {
        return find_bpe_pairs_scalar(text, pairs);
    }
    
    std::vector<size_t> positions;
    const char* ptr = text.data();
    size_t len = text.length();
    
    #ifdef R3M_SIMD_X86_AVAILABLE
        if (supports_avx2()) {
            for (const auto& pair : pairs) {
                if (pair.length() != 2) continue; // Only handle 2-character pairs
                
                char first = pair[0], second = pair[1];
                __m256i first_vec = _mm256_set1_epi8(first);
                __m256i second_vec = _mm256_set1_epi8(second);
                
                size_t i = 0;
                for (; i + 32 <= len - 1; i += 32) {
                    __m256i chunk = _mm256_loadu_si256((__m256i*)(ptr + i));
                    __m256i next_chunk = _mm256_loadu_si256((__m256i*)(ptr + i + 1));
                    
                    __m256i first_match = _mm256_cmpeq_epi8(chunk, first_vec);
                    __m256i second_match = _mm256_cmpeq_epi8(next_chunk, second_vec);
                    __m256i pair_match = _mm256_and_si256(first_match, second_match);
                    
                    int mask = _mm256_movemask_epi8(pair_match);
                    while (mask != 0) {
                        int pos = __builtin_ctz(mask);
                        positions.push_back(i + pos);
                        mask &= mask - 1; // Clear lowest set bit
                    }
                }
                
                // Scalar fallback for remaining characters
                for (; i < len - 1; ++i) {
                    if (ptr[i] == first && ptr[i + 1] == second) {
                        positions.push_back(i);
                    }
                }
            }
        } else {
            return find_bpe_pairs_scalar(text, pairs);
        }
    #elif defined(R3M_SIMD_ARM_AVAILABLE)
        for (const auto& pair : pairs) {
            if (pair.length() != 2) continue;
            
            char first = pair[0], second = pair[1];
            uint8x16_t first_vec = vdupq_n_u8(first);
            uint8x16_t second_vec = vdupq_n_u8(second);
            
            size_t i = 0;
            for (; i + 16 <= len - 1; i += 16) {
                uint8x16_t chunk = vld1q_u8((uint8_t*)(ptr + i));
                uint8x16_t next_chunk = vld1q_u8((uint8_t*)(ptr + i + 1));
                
                uint8x16_t first_match = vceqq_u8(chunk, first_vec);
                uint8x16_t second_match = vceqq_u8(next_chunk, second_vec);
                uint8x16_t pair_match = vandq_u8(first_match, second_match);
                
                uint64x2_t mask = vreinterpretq_u64_u8(pair_match);
                uint64_t mask_low = vgetq_lane_u64(mask, 0);
                uint64_t mask_high = vgetq_lane_u64(mask, 1);
                
                // Process low 64 bits
                while (mask_low != 0) {
                    int pos = __builtin_ctzll(mask_low);
                    positions.push_back(i + pos);
                    mask_low &= mask_low - 1;
                }
                
                // Process high 64 bits
                while (mask_high != 0) {
                    int pos = __builtin_ctzll(mask_high);
                    positions.push_back(i + pos + 8);
                    mask_high &= mask_high - 1;
                }
            }
            
            // Scalar fallback for remaining characters
            for (; i < len - 1; ++i) {
                if (ptr[i] == first && ptr[i + 1] == second) {
                    positions.push_back(i);
                }
            }
        }
    #else
        return find_bpe_pairs_scalar(text, pairs);
    #endif
    
    return positions;
}

// Fast sentence boundary detection
std::vector<size_t> SIMDUtils::find_sentence_boundaries_simd(const std::string& text) {
    if (!supports_simd()) {
        return find_sentence_boundaries_scalar(text);
    }
    
    std::vector<size_t> boundaries;
    const char* ptr = text.data();
    size_t len = text.length();
    
    #ifdef R3M_SIMD_X86_AVAILABLE
        if (supports_avx2()) {
            // Create vectors for sentence delimiters
            __m256i period_vec = _mm256_set1_epi8('.');
            __m256i exclamation_vec = _mm256_set1_epi8('!');
            __m256i question_vec = _mm256_set1_epi8('?');
            __m256i newline_vec = _mm256_set1_epi8('\n');
            
            size_t i = 0;
            for (; i + 32 <= len; i += 32) {
                __m256i chunk = _mm256_loadu_si256((__m256i*)(ptr + i));
                
                __m256i period_match = _mm256_cmpeq_epi8(chunk, period_vec);
                __m256i exclamation_match = _mm256_cmpeq_epi8(chunk, exclamation_vec);
                __m256i question_match = _mm256_cmpeq_epi8(chunk, question_vec);
                __m256i newline_match = _mm256_cmpeq_epi8(chunk, newline_vec);
                
                __m256i delimiter_match = _mm256_or_si256(
                    _mm256_or_si256(period_match, exclamation_match),
                    _mm256_or_si256(question_match, newline_match)
                );
                
                int mask = _mm256_movemask_epi8(delimiter_match);
                while (mask != 0) {
                    int pos = __builtin_ctz(mask);
                    boundaries.push_back(i + pos);
                    mask &= mask - 1;
                }
            }
            
            // Scalar fallback for remaining characters
            for (; i < len; ++i) {
                char c = ptr[i];
                if (c == '.' || c == '!' || c == '?' || c == '\n') {
                    boundaries.push_back(i);
                }
            }
        } else {
            return find_sentence_boundaries_scalar(text);
        }
    #elif defined(R3M_SIMD_ARM_AVAILABLE)
        uint8x16_t period_vec = vdupq_n_u8('.');
        uint8x16_t exclamation_vec = vdupq_n_u8('!');
        uint8x16_t question_vec = vdupq_n_u8('?');
        uint8x16_t newline_vec = vdupq_n_u8('\n');
        
        size_t i = 0;
        for (; i + 16 <= len; i += 16) {
            uint8x16_t chunk = vld1q_u8((uint8_t*)(ptr + i));
            
            uint8x16_t period_match = vceqq_u8(chunk, period_vec);
            uint8x16_t exclamation_match = vceqq_u8(chunk, exclamation_vec);
            uint8x16_t question_match = vceqq_u8(chunk, question_vec);
            uint8x16_t newline_match = vceqq_u8(chunk, newline_vec);
            
            uint8x16_t delimiter_match = vorrq_u8(
                vorrq_u8(period_match, exclamation_match),
                vorrq_u8(question_match, newline_match)
            );
            
            uint64x2_t mask = vreinterpretq_u64_u8(delimiter_match);
            uint64_t mask_low = vgetq_lane_u64(mask, 0);
            uint64_t mask_high = vgetq_lane_u64(mask, 1);
            
            // Process low 64 bits
            while (mask_low != 0) {
                int pos = __builtin_ctzll(mask_low);
                boundaries.push_back(i + pos);
                mask_low &= mask_low - 1;
            }
            
            // Process high 64 bits
            while (mask_high != 0) {
                int pos = __builtin_ctzll(mask_high);
                boundaries.push_back(i + pos + 8);
                mask_high &= mask_high - 1;
            }
        }
        
        // Scalar fallback for remaining characters
        for (; i < len; ++i) {
            char c = ptr[i];
            if (c == '.' || c == '!' || c == '?' || c == '\n') {
                boundaries.push_back(i);
            }
        }
    #else
        return find_sentence_boundaries_scalar(text);
    #endif
    
    return boundaries;
}

// Multi-character pattern matching
std::vector<size_t> SIMDUtils::find_pattern_simd(const std::string& text, const std::string& pattern) {
    if (!supports_simd() || pattern.length() < 2) {
        return find_pattern_scalar(text, pattern);
    }
    
    std::vector<size_t> positions;
    const char* ptr = text.data();
    size_t len = text.length();
    size_t pattern_len = pattern.length();
    
    if (len < pattern_len) return positions;
    
    #ifdef R3M_SIMD_X86_AVAILABLE
        if (supports_avx2()) {
            // For simplicity, implement 2-character pattern matching
            if (pattern_len == 2) {
                char first = pattern[0], second = pattern[1];
                __m256i first_vec = _mm256_set1_epi8(first);
                __m256i second_vec = _mm256_set1_epi8(second);
                
                size_t i = 0;
                for (; i + 32 <= len - 1; i += 32) {
                    __m256i chunk = _mm256_loadu_si256((__m256i*)(ptr + i));
                    __m256i next_chunk = _mm256_loadu_si256((__m256i*)(ptr + i + 1));
                    
                    __m256i first_match = _mm256_cmpeq_epi8(chunk, first_vec);
                    __m256i second_match = _mm256_cmpeq_epi8(next_chunk, second_vec);
                    __m256i pattern_match = _mm256_and_si256(first_match, second_match);
                    
                    int mask = _mm256_movemask_epi8(pattern_match);
                    while (mask != 0) {
                        int pos = __builtin_ctz(mask);
                        positions.push_back(i + pos);
                        mask &= mask - 1;
                    }
                }
                
                // Scalar fallback for remaining characters
                for (; i < len - 1; ++i) {
                    if (ptr[i] == first && ptr[i + 1] == second) {
                        positions.push_back(i);
                    }
                }
            }
        } else {
            return find_pattern_scalar(text, pattern);
        }
    #elif defined(R3M_SIMD_ARM_AVAILABLE)
        if (pattern_len == 2) {
            char first = pattern[0], second = pattern[1];
            uint8x16_t first_vec = vdupq_n_u8(first);
            uint8x16_t second_vec = vdupq_n_u8(second);
            
            size_t i = 0;
            for (; i + 16 <= len - 1; i += 16) {
                uint8x16_t chunk = vld1q_u8((uint8_t*)(ptr + i));
                uint8x16_t next_chunk = vld1q_u8((uint8_t*)(ptr + i + 1));
                
                uint8x16_t first_match = vceqq_u8(chunk, first_vec);
                uint8x16_t second_match = vceqq_u8(next_chunk, second_vec);
                uint8x16_t pattern_match = vandq_u8(first_match, second_match);
                
                uint64x2_t mask = vreinterpretq_u64_u8(pattern_match);
                uint64_t mask_low = vgetq_lane_u64(mask, 0);
                uint64_t mask_high = vgetq_lane_u64(mask, 1);
                
                // Process low 64 bits
                while (mask_low != 0) {
                    int pos = __builtin_ctzll(mask_low);
                    positions.push_back(i + pos);
                    mask_low &= mask_low - 1;
                }
                
                // Process high 64 bits
                while (mask_high != 0) {
                    int pos = __builtin_ctzll(mask_high);
                    positions.push_back(i + pos + 8);
                    mask_high &= mask_high - 1;
                }
            }
            
            // Scalar fallback for remaining characters
            for (; i < len - 1; ++i) {
                if (ptr[i] == first && ptr[i + 1] == second) {
                    positions.push_back(i);
                }
            }
        }
    #else
        return find_pattern_scalar(text, pattern);
    #endif
    
    return positions;
}

// Text normalization for vector search
std::string SIMDUtils::normalize_for_search_simd(const std::string& text) {
    if (!supports_simd()) {
        return normalize_for_search_scalar(text);
    }
    
    std::string result;
    result.reserve(text.length());
    const char* ptr = text.data();
    size_t len = text.length();
    
    #ifdef R3M_SIMD_X86_AVAILABLE
        if (supports_avx2()) {
            // Create vectors for characters to remove
            __m256i space_vec = _mm256_set1_epi8(' ');
            __m256i tab_vec = _mm256_set1_epi8('\t');
            __m256i newline_vec = _mm256_set1_epi8('\n');
            __m256i carriage_return_vec = _mm256_set1_epi8('\r');
            
            size_t i = 0;
            for (; i + 32 <= len; i += 32) {
                __m256i chunk = _mm256_loadu_si256((__m256i*)(ptr + i));
                
                __m256i space_match = _mm256_cmpeq_epi8(chunk, space_vec);
                __m256i tab_match = _mm256_cmpeq_epi8(chunk, tab_vec);
                __m256i newline_match = _mm256_cmpeq_epi8(chunk, newline_vec);
                __m256i cr_match = _mm256_cmpeq_epi8(chunk, carriage_return_vec);
                
                __m256i whitespace_match = _mm256_or_si256(
                    _mm256_or_si256(space_match, tab_match),
                    _mm256_or_si256(newline_match, cr_match)
                );
                
                int mask = _mm256_movemask_epi8(whitespace_match);
                
                // Only add non-whitespace characters
                for (int j = 0; j < 32; ++j) {
                    if (!(mask & (1 << j))) {
                        result += ptr[i + j];
                    }
                }
            }
            
            // Scalar fallback for remaining characters
            for (; i < len; ++i) {
                char c = ptr[i];
                if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                    result += c;
                }
            }
        } else {
            return normalize_for_search_scalar(text);
        }
    #elif defined(R3M_SIMD_ARM_AVAILABLE)
        uint8x16_t space_vec = vdupq_n_u8(' ');
        uint8x16_t tab_vec = vdupq_n_u8('\t');
        uint8x16_t newline_vec = vdupq_n_u8('\n');
        uint8x16_t carriage_return_vec = vdupq_n_u8('\r');
        
        size_t i = 0;
        for (; i + 16 <= len; i += 16) {
            uint8x16_t chunk = vld1q_u8((uint8_t*)(ptr + i));
            
            uint8x16_t space_match = vceqq_u8(chunk, space_vec);
            uint8x16_t tab_match = vceqq_u8(chunk, tab_vec);
            uint8x16_t newline_match = vceqq_u8(chunk, newline_vec);
            uint8x16_t cr_match = vceqq_u8(chunk, carriage_return_vec);
            
            uint8x16_t whitespace_match = vorrq_u8(
                vorrq_u8(space_match, tab_match),
                vorrq_u8(newline_match, cr_match)
            );
            
            uint64x2_t mask = vreinterpretq_u64_u8(whitespace_match);
            uint64_t mask_low = vgetq_lane_u64(mask, 0);
            uint64_t mask_high = vgetq_lane_u64(mask, 1);
            
            // Process low 64 bits
            for (int j = 0; j < 8; ++j) {
                if (!(mask_low & (1ULL << j))) {
                    result += ptr[i + j];
                }
            }
            
            // Process high 64 bits
            for (int j = 0; j < 8; ++j) {
                if (!(mask_high & (1ULL << j))) {
                    result += ptr[i + j + 8];
                }
            }
        }
        
        // Scalar fallback for remaining characters
        for (; i < len; ++i) {
            char c = ptr[i];
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                result += c;
            }
        }
    #else
        return normalize_for_search_scalar(text);
    #endif
    
    return result;
}

// Scalar fallback implementations
size_t SIMDUtils::count_char_scalar(const std::string& text, char target) {
    return std::count(text.begin(), text.end(), target);
}

size_t SIMDUtils::find_substring_scalar(const std::string& text, const std::string& pattern) {
    return text.find(pattern);
}

size_t SIMDUtils::count_whitespace_scalar(const std::string& text) {
    return std::count_if(text.begin(), text.end(), ::isspace);
}

size_t SIMDUtils::count_punctuation_scalar(const std::string& text) {
    return std::count_if(text.begin(), text.end(), ::ispunct);
}

std::string SIMDUtils::clean_text_scalar(const std::string& text, const std::vector<char>& chars_to_remove) {
    std::string result;
    result.reserve(text.length());
    
    for (char ch : text) {
        if (std::find(chars_to_remove.begin(), chars_to_remove.end(), ch) == chars_to_remove.end()) {
            result += ch;
        }
    }
    
    return result;
}

size_t SIMDUtils::count_tokens_scalar(const std::string& text) {
    std::istringstream iss(text);
    std::string token;
    size_t count = 0;
    
    while (iss >> token) {
        count++;
    }
    
    return count;
}

std::vector<std::string> SIMDUtils::split_by_delimiter_scalar(const std::string& text, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(text);
    std::string token;
    
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

// BPE and advanced text processing scalar fallbacks
std::vector<size_t> SIMDUtils::find_bpe_pairs_scalar(const std::string& text, const std::vector<std::string>& pairs) {
    std::vector<size_t> positions;
    
    for (const auto& pair : pairs) {
        if (pair.length() != 2) continue;
        
        char first = pair[0], second = pair[1];
        for (size_t i = 0; i < text.length() - 1; ++i) {
            if (text[i] == first && text[i + 1] == second) {
                positions.push_back(i);
            }
        }
    }
    
    return positions;
}

std::vector<size_t> SIMDUtils::find_sentence_boundaries_scalar(const std::string& text) {
    std::vector<size_t> boundaries;
    
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        if (c == '.' || c == '!' || c == '?' || c == '\n') {
            boundaries.push_back(i);
        }
    }
    
    return boundaries;
}

std::vector<size_t> SIMDUtils::find_pattern_scalar(const std::string& text, const std::string& pattern) {
    std::vector<size_t> positions;
    
    if (pattern.length() < 2) return positions;
    
    for (size_t i = 0; i <= text.length() - pattern.length(); ++i) {
        if (text.substr(i, pattern.length()) == pattern) {
            positions.push_back(i);
        }
    }
    
    return positions;
}

std::string SIMDUtils::normalize_for_search_scalar(const std::string& text) {
    std::string result;
    result.reserve(text.length());
    
    for (char c : text) {
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            result += c;
        }
    }
    
    return result;
}

} // namespace r3m::utils 