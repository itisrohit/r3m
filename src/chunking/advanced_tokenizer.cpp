#include "r3m/chunking/advanced_tokenizer.hpp"
#include "r3m/utils/text_processing.hpp"
#include "r3m/utils/simd_utils.hpp"
#include <sstream>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <functional>

namespace r3m::chunking {

// SimpleTokenizer implementation
std::vector<int> SimpleTokenizer::encode(const std::string& text) {
    auto tokens = split_text(text);
    std::vector<int> encoded;
    encoded.reserve(tokens.size());
    
    for (const auto& token : tokens) {
        encoded.push_back(hash_token(token));
    }
    
    return encoded;
}

std::vector<std::string> SimpleTokenizer::tokenize(const std::string& text) {
    return split_text(text);
}

std::string SimpleTokenizer::decode(const std::vector<int>& tokens) {
    // Simple implementation - in practice you'd maintain a reverse mapping
    std::string result;
    for (int token_id : tokens) {
        // For simplicity, we'll just use the token ID as a character
        result += static_cast<char>(token_id % 128);
    }
    return result;
}

int SimpleTokenizer::count_tokens(const std::string& text) {
    return static_cast<int>(split_text(text).size());
}

std::vector<std::string> SimpleTokenizer::split_text(const std::string& text) {
    std::vector<std::string> tokens;
    std::istringstream iss(text);
    std::string token;
    
    while (iss >> token) {
        // Clean the token
        token = r3m::utils::TextProcessing::clean_text(token);
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

int SimpleTokenizer::hash_token(const std::string& token) {
    std::hash<std::string> hasher;
    return static_cast<int>(hasher(token) % 1000000);
}

// SentenceTokenizer implementation
SentenceTokenizer::SentenceTokenizer(bool preserve_punctuation) 
    : preserve_punctuation_(preserve_punctuation),
      sentence_pattern_(R"([.!?]+[\s\n]*)"),
      word_pattern_(R"(\b\w+\b)") {
}

std::vector<int> SentenceTokenizer::encode(const std::string& text) {
    auto tokens = tokenize(text);
    std::vector<int> encoded;
    encoded.reserve(tokens.size());
    
    for (const auto& token : tokens) {
        encoded.push_back(hash_token(token));
    }
    
    return encoded;
}

std::vector<std::string> SentenceTokenizer::tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    
    // Split into sentences first
    auto sentences = split_sentences(text);
    
    for (const auto& sentence : sentences) {
        auto words = split_words(sentence);
        tokens.insert(tokens.end(), words.begin(), words.end());
        
        // Add sentence boundary token if preserving punctuation
        if (preserve_punctuation_ && !sentence.empty()) {
            char last_char = sentence.back();
            if (last_char == '.' || last_char == '!' || last_char == '?') {
                tokens.push_back(std::string(1, last_char));
            }
        }
    }
    
    return tokens;
}

std::string SentenceTokenizer::decode(const std::vector<int>& tokens) {
    std::string result;
    for (int token_id : tokens) {
        // Simple decoding - in practice you'd use a reverse vocabulary
        result += static_cast<char>(token_id % 128);
    }
    return result;
}

int SentenceTokenizer::count_tokens(const std::string& text) {
    return static_cast<int>(tokenize(text).size());
}

std::vector<std::string> SentenceTokenizer::split_sentences(const std::string& text) {
    std::vector<std::string> sentences;
    
    // Use SIMD-optimized sentence boundary detection
    auto boundaries = r3m::utils::SIMDUtils::find_sentence_boundaries_simd(text);
    
    if (boundaries.empty()) {
        // No sentence boundaries found, return the whole text as one sentence
        sentences.push_back(text);
        return sentences;
    }
    
    // Split text based on found boundaries
    size_t start = 0;
    for (size_t boundary : boundaries) {
        if (boundary > start) {
            std::string sentence = text.substr(start, boundary - start + 1);
            // Clean up the sentence
            sentence = r3m::utils::TextProcessing::clean_text(sentence);
            if (!sentence.empty()) {
                sentences.push_back(sentence);
            }
        }
        start = boundary + 1;
    }
    
    // Add remaining text as a sentence
    if (start < text.length()) {
        std::string sentence = text.substr(start);
        sentence = r3m::utils::TextProcessing::clean_text(sentence);
        if (!sentence.empty()) {
            sentences.push_back(sentence);
        }
    }
    
    return sentences;
}

std::vector<std::string> SentenceTokenizer::split_words(const std::string& text) {
    std::vector<std::string> words;
    std::istringstream iss(text);
    std::string word;
    
    while (iss >> word) {
        // Clean the word
        word = r3m::utils::TextProcessing::clean_text(word);
        if (!word.empty()) {
            words.push_back(word);
        }
    }
    
    return words;
}

int SentenceTokenizer::hash_token(const std::string& token) {
    std::hash<std::string> hasher;
    return static_cast<int>(hasher(token) % 1000000);
}

// BPETokenizer implementation
BPETokenizer::BPETokenizer(size_t vocab_size) : vocab_size_(vocab_size) {
    // Initialize with basic vocabulary
    for (int i = 0; i < 128; ++i) {
        std::string token = std::string(1, static_cast<char>(i));
        vocab_[token] = i;
        reverse_vocab_[i] = token;
    }
}

std::vector<int> BPETokenizer::encode(const std::string& text) {
    auto tokens = tokenize(text);
    std::vector<int> encoded;
    encoded.reserve(tokens.size());
    
    for (const auto& token : tokens) {
        auto it = vocab_.find(token);
        if (it != vocab_.end()) {
            encoded.push_back(it->second);
        } else {
            // Unknown token - use a default ID
            encoded.push_back(0);
        }
    }
    
    return encoded;
}

std::vector<std::string> BPETokenizer::tokenize(const std::string& text) {
    return byte_pair_encode(text);
}

std::string BPETokenizer::decode(const std::vector<int>& tokens) {
    std::vector<std::string> token_strings;
    token_strings.reserve(tokens.size());
    
    for (int token_id : tokens) {
        auto it = reverse_vocab_.find(token_id);
        if (it != reverse_vocab_.end()) {
            token_strings.push_back(it->second);
        } else {
            token_strings.push_back("<UNK>");
        }
    }
    
    return byte_pair_decode(token_strings);
}

int BPETokenizer::count_tokens(const std::string& text) {
    return static_cast<int>(tokenize(text).size());
}

void BPETokenizer::train(const std::vector<std::string>& corpus) {
    build_vocabulary(corpus);
}

void BPETokenizer::save_vocabulary(const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        for (const auto& [token, id] : vocab_) {
            file << id << "\t" << token << "\n";
        }
        file.close();
    }
}

void BPETokenizer::load_vocabulary(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        vocab_.clear();
        reverse_vocab_.clear();
        
        std::string line;
        while (std::getline(file, line)) {
            size_t tab_pos = line.find('\t');
            if (tab_pos != std::string::npos) {
                int id = std::stoi(line.substr(0, tab_pos));
                std::string token = line.substr(tab_pos + 1);
                vocab_[token] = id;
                reverse_vocab_[id] = token;
            }
        }
        file.close();
    }
}

void BPETokenizer::build_vocabulary(const std::vector<std::string>& corpus) {
    // Count character pairs using SIMD optimization
    pair_freqs_.clear();
    
    for (const auto& text : corpus) {
        // Extract all 2-character pairs from the vocabulary
        std::vector<std::string> pairs;
        for (const auto& [token, id] : vocab_) {
            if (token.length() == 2) {
                pairs.push_back(token);
            }
        }
        
        // Use SIMD-optimized pair finding
        auto positions = r3m::utils::SIMDUtils::find_bpe_pairs_simd(text, pairs);
        
        // Count pairs found by SIMD
        for (size_t pos : positions) {
            if (pos + 1 < text.length()) {
                std::string pair = text.substr(pos, 2);
                pair_freqs_[pair]++;
            }
        }
        
        // Also count adjacent character pairs for new vocabulary building
        for (size_t i = 0; i < text.length() - 1; ++i) {
            std::string pair = text.substr(i, 2);
            pair_freqs_[pair]++;
        }
    }
    
    // Build vocabulary through merges
    while (vocab_.size() < vocab_size_ && !pair_freqs_.empty()) {
        merge_pairs();
    }
}

std::vector<std::string> BPETokenizer::byte_pair_encode(const std::string& text) {
    std::vector<std::string> tokens;
    
    // Start with individual characters
    for (char ch : text) {
        tokens.push_back(std::string(1, ch));
    }
    
    // If vocabulary is empty, just return character tokens
    if (vocab_.size() <= 128) {
        return tokens;
    }
    
    // Extract all 2-character pairs from vocabulary for SIMD processing
    std::vector<std::string> pairs;
    for (const auto& [token, id] : vocab_) {
        if (token.length() == 2) {
            pairs.push_back(token);
        }
    }
    
    // Use SIMD-optimized pair finding for faster encoding
    auto positions = r3m::utils::SIMDUtils::find_bpe_pairs_simd(text, pairs);
    
    // Process found pairs in reverse order to avoid index shifting issues
    std::sort(positions.begin(), positions.end(), std::greater<size_t>());
    
    // Create a new tokens vector to avoid modifying while iterating
    std::vector<std::string> result_tokens;
    result_tokens.reserve(tokens.size());
    
    // Copy tokens and apply BPE merges
    for (size_t i = 0; i < tokens.size(); ++i) {
        bool merged = false;
        
        // Check if this position should be merged
        for (size_t pos : positions) {
            if (pos == i && pos + 1 < text.length()) {
                std::string pair = text.substr(pos, 2);
                auto it = vocab_.find(pair);
                if (it != vocab_.end()) {
                    result_tokens.push_back(pair);
                    i++; // Skip the next token since we merged it
                    merged = true;
                    break;
                }
            }
        }
        
        if (!merged) {
            result_tokens.push_back(tokens[i]);
        }
    }
    
    return result_tokens;
}

std::string BPETokenizer::byte_pair_decode(const std::vector<std::string>& tokens) {
    std::string result;
    for (const auto& token : tokens) {
        result += token;
    }
    return result;
}

void BPETokenizer::merge_pairs() {
    if (pair_freqs_.empty()) return;
    
    // Find the most frequent pair
    auto max_pair = std::max_element(pair_freqs_.begin(), pair_freqs_.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
    
    if (max_pair != pair_freqs_.end()) {
        std::string new_token = max_pair->first;
        int new_id = static_cast<int>(vocab_.size());
        
        vocab_[new_token] = new_id;
        reverse_vocab_[new_id] = new_token;
        
        // Remove the pair from frequencies
        pair_freqs_.erase(max_pair);
    }
}

// TokenizerFactory implementation
std::shared_ptr<AdvancedTokenizer> TokenizerFactory::create(Type type, const std::string& model_name) {
    (void)model_name; // Suppress unused parameter warning
    switch (type) {
        case Type::SIMPLE:
            return std::make_shared<SimpleTokenizer>();
        case Type::SENTENCE:
            return std::make_shared<SentenceTokenizer>();
        case Type::BPE:
            return std::make_shared<BPETokenizer>();
        default:
            return std::make_shared<SimpleTokenizer>();
    }
}

std::shared_ptr<AdvancedTokenizer> TokenizerFactory::create_from_config(const std::string& config) {
    // Simple config parsing - in practice you'd use a proper parser
    if (config.find("sentence") != std::string::npos) {
        return std::make_shared<SentenceTokenizer>();
    } else if (config.find("bpe") != std::string::npos) {
        return std::make_shared<BPETokenizer>();
    } else {
        return std::make_shared<SimpleTokenizer>();
    }
}

} // namespace r3m::chunking 