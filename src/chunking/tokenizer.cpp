#include "r3m/chunking/tokenizer.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace r3m {
namespace chunking {

BasicTokenizer::BasicTokenizer(size_t max_tokens) 
    : max_tokens_(max_tokens) {
}

size_t BasicTokenizer::count_tokens(const std::string& text) const {
    return tokenize(text).size();
}

std::vector<std::string> BasicTokenizer::encode(const std::string& text) const {
    return tokenize(text);
}

std::vector<std::string> BasicTokenizer::tokenize(const std::string& text) const {
    auto tokens = split_text(text);
    
    // Limit tokens to max_tokens_ if needed
    if (tokens.size() > max_tokens_) {
        tokens.resize(max_tokens_);
    }
    
    return tokens;
}

std::vector<std::string> BasicTokenizer::split_text(const std::string& text) const {
    std::vector<std::string> tokens;
    std::string current_token;
    
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        
        if (std::isspace(c)) {
            // End of current token
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
        } else if (is_punctuation(c)) {
            // End current token and add punctuation as separate token
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
            tokens.push_back(std::string(1, c));
        } else {
            // Add character to current token
            current_token += c;
        }
    }
    
    // Add final token if exists
    if (!current_token.empty()) {
        tokens.push_back(current_token);
    }
    
    return tokens;
}

bool BasicTokenizer::is_punctuation(char c) const {
    static const std::string punctuation = ".,!?;:()[]{}\"'`~@#$%^&*+=|\\/<>";
    return punctuation.find(c) != std::string::npos;
}

std::unique_ptr<Tokenizer> TokenizerFactory::create(Type type, size_t max_tokens) {
    switch (type) {
        case Type::BASIC:
            return std::make_unique<BasicTokenizer>(max_tokens);
        default:
            return std::make_unique<BasicTokenizer>(max_tokens);
    }
}

} // namespace chunking
} // namespace r3m 