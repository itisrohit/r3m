#include "r3m/utils/text_processing.hpp"
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

std::string TextProcessing::clean_text(const std::string& text) {
    std::string cleaned;
    cleaned.reserve(text.length());
    
    for (char ch : text) {
        // Check if character is in filtered Unicode ranges
        if (is_unicode_filtered(static_cast<uint32_t>(ch))) {
            continue;
        }
        
        // Keep control characters except newline and tab
        if (ch >= ' ' || ch == '\n' || ch == '\t') {
            cleaned += ch;
        }
    }
    
    return cleaned;
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
    std::string result;
    result.reserve(text.length());
    
    for (char ch : text) {
        if (!std::ispunct(ch)) {
            result += ch;
        }
    }
    
    return result;
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
    
    // URL encode the result
    return url_encode(result);
}

bool TextProcessing::has_unescaped_quote(const std::string& text) {
    std::regex pattern("(?<!\\\\)\"");
    return std::regex_search(text, pattern);
}

std::string TextProcessing::escape_quotes(const std::string& text) {
    std::string result;
    result.reserve(text.length() * 2);
    
    bool in_string = false;
    for (size_t i = 0; i < text.length(); ++i) {
        char ch = text[i];
        
        if (ch == '"') {
            if (!in_string) {
                in_string = true;
                result += ch;
            } else {
                // Check if previous character is backslash
                if (!result.empty() && result.back() == '\\') {
                    result += ch;
                } else {
                    // Check next character
                    char next_char = (i + 1 < text.length()) ? text[i + 1] : '\0';
                    if (next_char != ',' && next_char != ':' && next_char != '}' && next_char != '\n') {
                        result += "\\" + std::string(1, ch);
                    } else {
                        result += ch;
                        in_string = false;
                    }
                }
            }
        } else {
            result += ch;
        }
    }
    
    return result;
}

std::string TextProcessing::clean_up_code_blocks(const std::string& text) {
    std::string result = text;
    
    // Remove leading/trailing whitespace
    result.erase(0, result.find_first_not_of(" \t\n\r"));
    result.erase(result.find_last_not_of(" \t\n\r") + 1);
    
    // Remove markdown code block markers
    if (result.substr(0, 3) == "```") {
        result = result.substr(3);
    }
    if (result.substr(result.length() - 3) == "```") {
        result = result.substr(0, result.length() - 3);
    }
    
    // Remove leading/trailing whitespace again
    result.erase(0, result.find_first_not_of(" \t\n\r"));
    result.erase(result.find_last_not_of(" \t\n\r") + 1);
    
    // Replace \xa0 with space
    std::replace(result.begin(), result.end(), '\xa0', ' ');
    
    return result;
}

std::string TextProcessing::clean_model_quote(const std::string& quote, int trim_length) {
    std::string result = quote;
    
    // Remove leading/trailing whitespace
    result.erase(0, result.find_first_not_of(" \t\n\r"));
    result.erase(result.find_last_not_of(" \t\n\r") + 1);
    
    // Remove surrounding quotes
    if (!result.empty() && result[0] == '"') {
        result = result.substr(1);
    }
    if (!result.empty() && result.back() == '"') {
        result.pop_back();
    }
    
    // Trim if specified
    if (trim_length > 0 && static_cast<int>(result.length()) > trim_length) {
        result = result.substr(0, trim_length);
    }
    
    return result;
}

bool TextProcessing::is_valid_email(const std::string& text) {
    std::regex email_pattern(R"(^[a-zA-Z0-9._-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return std::regex_match(text, email_pattern);
}

int TextProcessing::count_punctuation(const std::string& text) {
    int count = 0;
    for (char ch : text) {
        if (std::ispunct(ch)) {
            count++;
        }
    }
    return count;
}

std::string TextProcessing::decode_escapes(const std::string& text) {
    // Simple implementation - in a real system you'd want a more robust Unicode decoder
    std::string result = text;
    
    // Handle common escape sequences
    std::unordered_map<std::string, std::string> escapes = {
        {"\\n", "\n"}, {"\\t", "\t"}, {"\\r", "\r"}, {"\\\"", "\""},
        {"\\\\", "\\"}, {"\\'", "'"}, {"\\a", "\a"}, {"\\b", "\b"},
        {"\\f", "\f"}, {"\\v", "\v"}
    };
    
    for (const auto& [escape, replacement] : escapes) {
        size_t pos = 0;
        while ((pos = result.find(escape, pos)) != std::string::npos) {
            result.replace(pos, escape.length(), replacement);
            pos += replacement.length();
        }
    }
    
    return result;
}

std::string TextProcessing::extract_embedded_json(const std::string& text) {
    size_t first_brace = text.find('{');
    size_t last_brace = text.rfind('}');
    
    if (first_brace == std::string::npos || last_brace == std::string::npos) {
        return "{\"answer\": \"" + text + "\", \"quotes\": []}";
    }
    
    return text.substr(first_brace, last_brace - first_brace + 1);
}

bool TextProcessing::is_unicode_filtered(uint32_t code_point) {
    for (const auto& [start, end] : UNICODE_FILTER_RANGES) {
        if (code_point >= start && code_point <= end) {
            return true;
        }
    }
    return false;
}

std::string TextProcessing::url_encode(const std::string& text) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    
    for (char ch : text) {
        if (std::isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
            escaped << ch;
        } else {
            escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(ch));
        }
    }
    
    return escaped.str();
}

} // namespace r3m::utils 