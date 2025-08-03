#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <regex>

namespace r3m::chunking {

/**
 * Advanced tokenizer interface
 */
class AdvancedTokenizer {
public:
    virtual ~AdvancedTokenizer() = default;
    
    /**
     * Encode text to token IDs
     */
    virtual std::vector<int> encode(const std::string& text) = 0;
    
    /**
     * Tokenize text to token strings
     */
    virtual std::vector<std::string> tokenize(const std::string& text) = 0;
    
    /**
     * Decode token IDs back to text
     */
    virtual std::string decode(const std::vector<int>& tokens) = 0;
    
    /**
     * Get token count for text
     */
    virtual int count_tokens(const std::string& text) = 0;
};

/**
 * Simple whitespace-based tokenizer (fallback)
 */
class SimpleTokenizer : public AdvancedTokenizer {
public:
    std::vector<int> encode(const std::string& text) override;
    std::vector<std::string> tokenize(const std::string& text) override;
    std::string decode(const std::vector<int>& tokens) override;
    int count_tokens(const std::string& text) override;

private:
    std::vector<std::string> split_text(const std::string& text);
    int hash_token(const std::string& token);
};

/**
 * Sentence-aware tokenizer with punctuation handling
 */
class SentenceTokenizer : public AdvancedTokenizer {
public:
    SentenceTokenizer(bool preserve_punctuation = true);
    
    std::vector<int> encode(const std::string& text) override;
    std::vector<std::string> tokenize(const std::string& text) override;
    std::string decode(const std::vector<int>& tokens) override;
    int count_tokens(const std::string& text) override;

private:
    bool preserve_punctuation_;
    std::regex sentence_pattern_;
    std::regex word_pattern_;
    
    std::vector<std::string> split_sentences(const std::string& text);
    std::vector<std::string> split_words(const std::string& text);
    int hash_token(const std::string& token);
};

/**
 * BPE-style tokenizer with vocabulary
 */
class BPETokenizer : public AdvancedTokenizer {
public:
    BPETokenizer(size_t vocab_size = 50000);
    
    std::vector<int> encode(const std::string& text) override;
    std::vector<std::string> tokenize(const std::string& text) override;
    std::string decode(const std::vector<int>& tokens) override;
    int count_tokens(const std::string& text) override;
    
    /**
     * Train the tokenizer on a corpus
     */
    void train(const std::vector<std::string>& corpus);
    
    /**
     * Save/load vocabulary
     */
    void save_vocabulary(const std::string& filename);
    void load_vocabulary(const std::string& filename);

private:
    size_t vocab_size_;
    std::unordered_map<std::string, int> vocab_;
    std::unordered_map<int, std::string> reverse_vocab_;
    std::unordered_map<std::string, int> pair_freqs_;
    
    void build_vocabulary(const std::vector<std::string>& corpus);
    std::vector<std::string> byte_pair_encode(const std::string& text);
    std::string byte_pair_decode(const std::vector<std::string>& tokens);
    void merge_pairs();
};

/**
 * Factory for creating tokenizers
 */
class TokenizerFactory {
public:
    enum class Type {
        SIMPLE,
        SENTENCE,
        BPE
    };
    
    static std::shared_ptr<AdvancedTokenizer> create(Type type, 
                                                    const std::string& model_name = "");
    
    /**
     * Create tokenizer from configuration
     */
    static std::shared_ptr<AdvancedTokenizer> create_from_config(const std::string& config);
};

} // namespace r3m::chunking 