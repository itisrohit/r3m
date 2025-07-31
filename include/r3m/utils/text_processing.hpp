#pragma once

#include <string>
#include <vector>
#include <regex>
#include <unordered_set>
#include <cctype>
#include <sstream>
#include <algorithm>

namespace r3m::utils {

/**
 * Advanced text processing utilities
 */
class TextProcessing {
public:
    // Text processing constants
    static constexpr const char* RETURN_SEPARATOR = "\n\r\n";
    static constexpr const char* SECTION_SEPARATOR = "\n\n";
    static constexpr const char* INDEX_SEPARATOR = "===";

    // Unicode ranges to filter
    static const std::vector<std::pair<uint32_t, uint32_t>> UNICODE_FILTER_RANGES;

    /**
     * Clean text by removing problematic Unicode characters and control characters
     */
    static std::string clean_text(const std::string& text);

    /**
     * Clean text for comparison by removing whitespace, punctuation, and special chars
     */
    static std::string shared_precompare_cleanup(const std::string& text);

    /**
     * Remove punctuation from text
     */
    static std::string remove_punctuation(const std::string& text);

    /**
     * Replace all whitespace characters with single spaces
     */
    static std::string replace_whitespaces_with_space(const std::string& text);

    /**
     * Escape newlines in text
     */
    static std::string escape_newlines(const std::string& text);

    /**
     * Make text URL-compatible by replacing spaces with underscores and encoding
     */
    static std::string make_url_compatible(const std::string& text);

    /**
     * Check if text contains unescaped quotes
     */
    static bool has_unescaped_quote(const std::string& text);

    /**
     * Escape quotes in JSON strings
     */
    static std::string escape_quotes(const std::string& text);

    /**
     * Clean up code blocks by removing markdown formatting
     */
    static std::string clean_up_code_blocks(const std::string& text);

    /**
     * Clean model quotes by removing surrounding quotes and trimming
     */
    static std::string clean_model_quote(const std::string& quote, int trim_length = 0);

    /**
     * Validate email format
     */
    static bool is_valid_email(const std::string& text);

    /**
     * Count punctuation characters in text
     */
    static int count_punctuation(const std::string& text);

    /**
     * Decode escape sequences in text
     */
    static std::string decode_escapes(const std::string& text);

    /**
     * Extract embedded JSON from text
     */
    static std::string extract_embedded_json(const std::string& text);

private:
    static bool is_unicode_filtered(uint32_t code_point);
    static std::string url_encode(const std::string& text);
};

} // namespace r3m::utils 