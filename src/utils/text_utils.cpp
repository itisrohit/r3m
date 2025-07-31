#include "r3m/utils/text_utils.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <set>

namespace r3m {
namespace utils {

// Text cleaning and normalization
std::string TextUtils::normalize_whitespace(const std::string& text) {
    std::string result = text;
    
    // Replace multiple whitespace with single space
    std::regex whitespace_regex("\\s+");
    result = std::regex_replace(result, whitespace_regex, " ");
    
    // Trim leading and trailing whitespace
    result.erase(0, result.find_first_not_of(" \t\n\r"));
    result.erase(result.find_last_not_of(" \t\n\r") + 1);
    
    return result;
}

std::string TextUtils::remove_html_tags(const std::string& text) {
    std::regex html_tag_regex("<[^>]*>");
    return std::regex_replace(text, html_tag_regex, "");
}

std::string TextUtils::clean_text_content(const std::string& text) {
    std::string result = text;
    
    // Remove null characters
    result.erase(std::remove(result.begin(), result.end(), '\0'), result.end());
    
    // Remove other control characters except newlines and tabs
    result.erase(std::remove_if(result.begin(), result.end(),
        [](char c) { return c < 32 && c != '\n' && c != '\t' && c != '\r'; }), result.end());
    
    return result;
}

std::string TextUtils::trim_whitespace(const std::string& text) {
    std::string result = text;
    result.erase(0, result.find_first_not_of(" \t\n\r"));
    result.erase(result.find_last_not_of(" \t\n\r") + 1);
    return result;
}

// Text analysis
std::vector<std::string> TextUtils::split_into_words(const std::string& text) {
    std::vector<std::string> words;
    std::stringstream ss(text);
    std::string word;
    
    while (ss >> word) {
        word = clean_word(word);
        if (!word.empty()) {
            words.push_back(word);
        }
    }
    
    return words;
}

std::set<std::string> TextUtils::get_unique_words(const std::string& text) {
    std::set<std::string> unique_words;
    std::stringstream ss(text);
    std::string word;
    
    while (ss >> word) {
        word = clean_word(word);
        if (!word.empty()) {
            unique_words.insert(word);
        }
    }
    
    return unique_words;
}

size_t TextUtils::count_sentences(const std::string& text) {
    size_t count = 0;
    for (char c : text) {
        if (c == '.' || c == '!' || c == '?') {
            count++;
        }
    }
    return count;
}

size_t TextUtils::count_technical_terms(const std::string& text) {
    size_t count = 0;
    std::stringstream ss(text);
    std::string word;
    
    while (ss >> word) {
        if (is_technical_word(word)) {
            count++;
        }
    }
    
    return count;
}

double TextUtils::calculate_word_diversity(const std::string& text) {
    auto unique_words = get_unique_words(text);
    auto all_words = split_into_words(text);
    
    if (all_words.empty()) {
        return 0.0;
    }
    
    return static_cast<double>(unique_words.size()) / all_words.size();
}

// Text validation
bool TextUtils::is_valid_text(const std::string& text) {
    return !text.empty() && !is_mostly_whitespace(text);
}

bool TextUtils::contains_technical_content(const std::string& text) {
    return count_technical_terms(text) > 0;
}

bool TextUtils::is_mostly_whitespace(const std::string& text) {
    size_t non_whitespace = 0;
    for (char c : text) {
        if (!std::isspace(c)) {
            non_whitespace++;
        }
    }
    return non_whitespace < text.length() * 0.1; // Less than 10% non-whitespace
}

// Encoding and format utilities
std::string TextUtils::detect_encoding(const std::string& file_path) {
    (void)file_path; // Suppress unused parameter warning
    return "utf-8"; // Simplified for now
}

bool TextUtils::is_utf8_valid(const std::string& text) {
    (void)text; // Suppress unused parameter warning
    return true; // Simplified for now
}

std::string TextUtils::convert_to_utf8(const std::string& text, const std::string& encoding) {
    (void)encoding; // Suppress unused parameter warning
    return text; // Simplified for now
}

// File utilities
std::string TextUtils::get_file_extension(const std::string& file_path) {
    std::filesystem::path path(file_path);
    return path.extension().string();
}

std::string TextUtils::get_file_name(const std::string& file_path) {
    std::filesystem::path path(file_path);
    return path.filename().string();
}

std::string TextUtils::get_file_directory(const std::string& file_path) {
    std::filesystem::path path(file_path);
    return path.parent_path().string();
}

bool TextUtils::file_exists(const std::string& file_path) {
    return std::filesystem::exists(file_path);
}

size_t TextUtils::get_file_size(const std::string& file_path) {
    if (std::filesystem::exists(file_path)) {
        return std::filesystem::file_size(file_path);
    }
    return 0;
}

// String utilities
std::string TextUtils::to_lowercase(const std::string& text) {
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string TextUtils::to_uppercase(const std::string& text) {
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool TextUtils::starts_with(const std::string& text, const std::string& prefix) {
    if (text.length() < prefix.length()) {
        return false;
    }
    return text.substr(0, prefix.length()) == prefix;
}

bool TextUtils::ends_with(const std::string& text, const std::string& suffix) {
    if (text.length() < suffix.length()) {
        return false;
    }
    return text.substr(text.length() - suffix.length()) == suffix;
}

std::string TextUtils::replace_all(const std::string& text, const std::string& from, const std::string& to) {
    std::string result = text;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

// Helper methods
bool TextUtils::is_technical_word(const std::string& word) {
    // Check for words with numbers, special characters, or technical patterns
    bool has_digit = std::any_of(word.begin(), word.end(), ::isdigit);
    bool has_special = std::any_of(word.begin(), word.end(), 
        [](char c) { return c == '_' || c == '-' || c == '.' || c == '#' || c == '@'; });
    
    return has_digit || has_special;
}

std::string TextUtils::clean_word(const std::string& word) {
    std::string cleaned = word;
    
    // Remove non-alphanumeric characters from the beginning and end
    while (!cleaned.empty() && !std::isalnum(cleaned.front())) {
        cleaned.erase(0, 1);
    }
    while (!cleaned.empty() && !std::isalnum(cleaned.back())) {
        cleaned.pop_back();
    }
    
    return cleaned;
}

} // namespace utils
} // namespace r3m 