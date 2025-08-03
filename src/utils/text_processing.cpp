#include "r3m/utils/text_processing.hpp"
#include "r3m/utils/simd_utils.hpp"
#include <iostream>
#include <iomanip>
#include <cstring>

namespace r3m::utils {

// Unicode filter ranges
const std::vector<std::pair<uint32_t, uint32_t>> TextProcessing::UNICODE_FILTER_RANGES = {
    {0x0000fff0, 0x0000ffff},  // Specials
    {0x0001f000, 0x0001f9ff},  // Emoticons
    {0x00002000, 0x0000206f},  // General Punctuation
    {0x00002190, 0x000021ff},  // Arrows
    {0x00002700, 0x000027bf}   // Dingbats
};

// Constants
const std::string TextProcessing::SECTION_SEPARATOR = "\n\n";

std::string TextProcessing::clean_text(const std::string& text) {
    // Use SIMD-optimized cleaning for better performance
    std::vector<char> chars_to_remove;
    
    // Add characters that should be removed based on Unicode filter ranges
    for (char ch = 0; ch < 127; ++ch) {
        if (is_unicode_filtered(static_cast<uint32_t>(ch))) {
            chars_to_remove.push_back(ch);
        }
    }
    
    // Use SIMD-optimized cleaning
    std::string cleaned = SIMDUtils::clean_text_simd(text, chars_to_remove);
    
    // Additional cleaning for control characters
    std::string result;
    result.reserve(cleaned.length());
    
    for (char ch : cleaned) {
        // Keep control characters except newline and tab
        if (ch >= ' ' || ch == '\n' || ch == '\t') {
            result += ch;
        }
    }
    
    return result;
}

std::string TextProcessing::shared_precompare_cleanup(const std::string& text) {
    std::string result = text;
    
    // Convert to lowercase
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    
    // Remove whitespace, asterisks, escaped quotes, and punctuation
    std::regex pattern(R"(\s|\*|\\"|[.,:`"#-])");
    result = std::regex_replace(result, pattern, "");
    
    return result;
}

std::string TextProcessing::remove_punctuation(const std::string& text) {
    // Use SIMD-optimized punctuation removal
    std::vector<char> punctuation_chars = {'.', ',', '!', '?', ';', ':', '"', '\'', '(', ')', '[', ']', '{', '}', '-', '_'};
    return SIMDUtils::clean_text_simd(text, punctuation_chars);
}

std::string TextProcessing::replace_whitespaces_with_space(const std::string& text) {
    std::string result;
    result.reserve(text.length());
    
    bool last_was_space = false;
    for (char ch : text) {
        if (std::isspace(ch)) {
            if (!last_was_space) {
                result += ' ';
                last_was_space = true;
            }
        } else {
            result += ch;
            last_was_space = false;
        }
    }
    
    return result;
}

std::string TextProcessing::escape_newlines(const std::string& text) {
    std::string result;
    result.reserve(text.length() * 2);
    
    for (size_t i = 0; i < text.length(); ++i) {
        if (text[i] == '\n' && (i == 0 || text[i-1] != '\\')) {
            result += "\\\\n";
        } else {
            result += text[i];
        }
    }
    
    return result;
}

std::string TextProcessing::make_url_compatible(const std::string& text) {
    std::string result = text;
    
    // Replace spaces with underscores
    std::replace(result.begin(), result.end(), ' ', '_');
    
    // Remove special characters
    std::vector<char> special_chars = {'<', '>', ':', '"', '/', '\\', '|', '?', '*'};
    result = SIMDUtils::clean_text_simd(result, special_chars);
    
    return result;
}

std::string TextProcessing::extract_blurb(const std::string& text, int max_length) {
    if (text.length() <= static_cast<size_t>(max_length)) {
        return text;
    }
    
    // Find the last space before max_length
    size_t cut_pos = text.find_last_of(' ', max_length);
    if (cut_pos == std::string::npos) {
        cut_pos = max_length;
    }
    
    return text.substr(0, cut_pos) + "...";
}

std::string TextProcessing::normalize_whitespace(const std::string& text) {
    std::string result;
    result.reserve(text.length());
    
    bool last_was_space = false;
    for (char ch : text) {
        if (std::isspace(ch)) {
            if (!last_was_space) {
                result += ' ';
                last_was_space = true;
            }
        } else {
            result += ch;
            last_was_space = false;
        }
    }
    
    // Trim leading and trailing whitespace
    if (!result.empty() && result.front() == ' ') {
        result = result.substr(1);
    }
    if (!result.empty() && result.back() == ' ') {
        result = result.substr(0, result.length() - 1);
    }
    
    return result;
}

std::string TextProcessing::remove_control_characters(const std::string& text) {
    std::string result;
    result.reserve(text.length());
    
    for (char ch : text) {
        if (ch >= ' ' || ch == '\n' || ch == '\t' || ch == '\r') {
            result += ch;
        }
    }
    
    return result;
}

std::string TextProcessing::clean_model_quote(const std::string& quote, int trim_length) {
    std::string cleaned = quote;
    
    // Remove common model prefixes/suffixes
    std::vector<std::string> prefixes = {"Assistant:", "Human:", "User:", "AI:", "Bot:"};
    std::vector<std::string> suffixes = {"\n\n", "\n", "..."};
    
    for (const auto& prefix : prefixes) {
        if (cleaned.substr(0, prefix.length()) == prefix) {
            cleaned = cleaned.substr(prefix.length());
            break;
        }
    }
    
    for (const auto& suffix : suffixes) {
        if (cleaned.length() >= suffix.length() && 
            cleaned.substr(cleaned.length() - suffix.length()) == suffix) {
            cleaned = cleaned.substr(0, cleaned.length() - suffix.length());
        }
    }
    
    // Trim whitespace
    cleaned = normalize_whitespace(cleaned);
    
    // Truncate if too long
    if (trim_length > 0 && static_cast<int>(cleaned.length()) > trim_length) {
        cleaned = extract_blurb(cleaned, trim_length);
    }
    
    return cleaned;
}

std::string TextProcessing::extract_text_from_html(const std::string& html) {
    std::string result;
    result.reserve(html.length());
    
    bool in_tag = false;
    bool in_script = false;
    bool in_style = false;
    
    for (size_t i = 0; i < html.length(); ++i) {
        char ch = html[i];
        
        if (ch == '<') {
            in_tag = true;
            // Check for script or style tags
            if (i + 6 < html.length() && html.substr(i + 1, 5) == "script") {
                in_script = true;
            } else if (i + 5 < html.length() && html.substr(i + 1, 4) == "style") {
                in_style = true;
            }
        } else if (ch == '>') {
            in_tag = false;
            // Check for closing script or style tags
            if (i >= 8 && html.substr(i - 7, 8) == "</script>") {
                in_script = false;
            } else if (i >= 7 && html.substr(i - 6, 7) == "</style>") {
                in_style = false;
            }
        } else if (!in_tag && !in_script && !in_style) {
            result += ch;
        }
    }
    
    return normalize_whitespace(result);
}

std::string TextProcessing::extract_text_from_markdown(const std::string& markdown) {
    std::string result = markdown;
    
    // Remove markdown formatting
    std::vector<std::pair<std::string, std::string>> replacements = {
        {"**", ""}, {"*", ""}, {"__", ""}, {"_", ""},
        {"#", ""}, {"##", ""}, {"###", ""}, {"####", ""},
        {"```", ""}, {"`", ""},
        {"[", ""}, {"]", ""}, {"(", ""}, {")", ""},
        {"!", ""}, {"|", ""}, {"-", ""}, {"+", ""}
    };
    
    for (const auto& [from, to] : replacements) {
        size_t pos = 0;
        while ((pos = result.find(from, pos)) != std::string::npos) {
            result.replace(pos, from.length(), to);
            pos += to.length();
        }
    }
    
    return normalize_whitespace(result);
}

bool TextProcessing::is_unicode_filtered(uint32_t code_point) {
    for (const auto& range : UNICODE_FILTER_RANGES) {
        if (code_point >= range.first && code_point <= range.second) {
            return true;
        }
    }
    return false;
}

// SIMD-optimized counting functions
size_t TextProcessing::count_characters(const std::string& text) {
    return SIMDUtils::count_char_simd(text, '\0');  // Count all characters
}

size_t TextProcessing::count_words(const std::string& text) {
    return SIMDUtils::count_tokens_simd(text);
}

size_t TextProcessing::count_sentences(const std::string& text) {
    size_t count = 0;
    for (char ch : text) {
        if (ch == '.' || ch == '!' || ch == '?') {
            count++;
        }
    }
    return count;
}

size_t TextProcessing::count_paragraphs(const std::string& text) {
    size_t count = 0;
    bool in_paragraph = false;
    
    for (size_t i = 0; i < text.length(); ++i) {
        if (text[i] == '\n') {
            if (in_paragraph) {
                count++;
                in_paragraph = false;
            }
        } else if (!std::isspace(text[i])) {
            in_paragraph = true;
        }
    }
    
    if (in_paragraph) {
        count++;
    }
    
    return count;
}

double TextProcessing::calculate_readability_score(const std::string& text) {
    size_t words = count_words(text);
    size_t sentences = count_sentences(text);
    size_t characters = count_characters(text);
    
    if (words == 0 || sentences == 0) {
        return 0.0;
    }
    
    // Flesch Reading Ease formula
    double avg_sentence_length = static_cast<double>(words) / sentences;
    double avg_syllables_per_word = static_cast<double>(characters) / (words * 3.0);  // Approximation
    
    double score = 206.835 - (1.015 * avg_sentence_length) - (84.6 * avg_syllables_per_word);
    return std::max(0.0, std::min(100.0, score));
}

std::string TextProcessing::generate_summary(const std::string& text, int max_length) {
    if (text.length() <= static_cast<size_t>(max_length)) {
        return text;
    }
    
    // Simple summary: take first few sentences
    std::string summary;
    size_t pos = 0;
    
    while (pos < text.length() && summary.length() < static_cast<size_t>(max_length)) {
        size_t next_sentence = text.find_first_of(".!?", pos);
        if (next_sentence == std::string::npos) {
            next_sentence = text.length();
        }
        
        std::string sentence = text.substr(pos, next_sentence - pos + 1);
        if (summary.length() + sentence.length() <= static_cast<size_t>(max_length)) {
            summary += sentence;
        } else {
            break;
        }
        
        pos = next_sentence + 1;
    }
    
    if (summary.empty()) {
        summary = extract_blurb(text, max_length);
    }
    
    return summary;
}

} // namespace r3m::utils 