#pragma once

#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include <set>

namespace r3m {
namespace utils {

class TextUtils {
public:
    // Text cleaning and normalization
    static std::string normalize_whitespace(const std::string& text);
    static std::string remove_html_tags(const std::string& text);
    static std::string clean_text_content(const std::string& text);
    static std::string trim_whitespace(const std::string& text);
    
    // Text analysis
    static std::vector<std::string> split_into_words(const std::string& text);
    static std::set<std::string> get_unique_words(const std::string& text);
    static size_t count_sentences(const std::string& text);
    static size_t count_technical_terms(const std::string& text);
    static double calculate_word_diversity(const std::string& text);
    
    // Text validation
    static bool is_valid_text(const std::string& text);
    static bool contains_technical_content(const std::string& text);
    static bool is_mostly_whitespace(const std::string& text);
    
    // Encoding and format utilities
    static std::string detect_encoding(const std::string& file_path);
    static bool is_utf8_valid(const std::string& text);
    static std::string convert_to_utf8(const std::string& text, const std::string& encoding);
    
    // File utilities
    static std::string get_file_extension(const std::string& file_path);
    static std::string get_file_name(const std::string& file_path);
    static std::string get_file_directory(const std::string& file_path);
    static bool file_exists(const std::string& file_path);
    static size_t get_file_size(const std::string& file_path);
    
    // String utilities
    static std::string to_lowercase(const std::string& text);
    static std::string to_uppercase(const std::string& text);
    static bool starts_with(const std::string& text, const std::string& prefix);
    static bool ends_with(const std::string& text, const std::string& suffix);
    static std::string replace_all(const std::string& text, const std::string& from, const std::string& to);
    
private:
    // Helper methods
    static bool is_technical_word(const std::string& word);
    static std::string clean_word(const std::string& word);
};

} // namespace utils
} // namespace r3m 