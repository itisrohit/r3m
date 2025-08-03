#pragma once

#include <string>
#include <vector>
#include <regex>
#include <utility>

namespace r3m::utils {

class TextProcessing {
public:
    // Unicode filter ranges for cleaning
    static const std::vector<std::pair<uint32_t, uint32_t>> UNICODE_FILTER_RANGES;
    
    // Core text processing functions
    static std::string clean_text(const std::string& text);
    static std::string shared_precompare_cleanup(const std::string& text);
    static std::string remove_punctuation(const std::string& text);
    static std::string replace_whitespaces_with_space(const std::string& text);
    static std::string escape_newlines(const std::string& text);
    static std::string make_url_compatible(const std::string& text);
    
    // Text extraction and formatting
    static std::string extract_blurb(const std::string& text, int max_length);
    static std::string normalize_whitespace(const std::string& text);
    static std::string remove_control_characters(const std::string& text);
    static std::string clean_model_quote(const std::string& quote, int trim_length);
    
    // HTML and Markdown processing
    static std::string extract_text_from_html(const std::string& html);
    static std::string extract_text_from_markdown(const std::string& markdown);
    
    // Unicode utilities
    static bool is_unicode_filtered(uint32_t code_point);
    
    // SIMD-optimized counting functions
    static size_t count_characters(const std::string& text);
    static size_t count_words(const std::string& text);
    static size_t count_sentences(const std::string& text);
    static size_t count_paragraphs(const std::string& text);
    
    // Text analysis
    static double calculate_readability_score(const std::string& text);
    static std::string generate_summary(const std::string& text, int max_length);
    
    // Constants
    static const std::string SECTION_SEPARATOR;
};

} // namespace r3m::utils 