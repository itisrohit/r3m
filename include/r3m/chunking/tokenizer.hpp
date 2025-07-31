#pragma once

#include <string>
#include <vector>
#include <memory>

namespace r3m {
namespace chunking {

/**
 * @brief Base tokenizer interface for chunking operations
 * 
 * Provides token counting and encoding capabilities for document processing.
 * This is the foundation for all chunking operations.
 */
class Tokenizer {
public:
    virtual ~Tokenizer() = default;
    
    /**
     * @brief Count tokens in text
     * @param text Input text to tokenize
     * @return Number of tokens
     */
    virtual size_t count_tokens(const std::string& text) const = 0;
    
    /**
     * @brief Encode text into tokens
     * @param text Input text
     * @return Vector of token strings
     */
    virtual std::vector<std::string> encode(const std::string& text) const = 0;
    
    /**
     * @brief Tokenize text into individual tokens
     * @param text Input text
     * @return Vector of token strings
     */
    virtual std::vector<std::string> tokenize(const std::string& text) const = 0;
    
    /**
     * @brief Get maximum tokens for this tokenizer
     * @return Maximum token limit
     */
    virtual size_t get_max_tokens() const = 0;
};

/**
 * @brief Basic word-based tokenizer implementation
 * 
 * Simple tokenizer that splits on whitespace and punctuation.
 * This is a fallback when more sophisticated tokenizers aren't available.
 */
class BasicTokenizer : public Tokenizer {
public:
    explicit BasicTokenizer(size_t max_tokens = 8192);
    ~BasicTokenizer() override = default;
    
    size_t count_tokens(const std::string& text) const override;
    std::vector<std::string> encode(const std::string& text) const override;
    std::vector<std::string> tokenize(const std::string& text) const override;
    size_t get_max_tokens() const override { return max_tokens_; }
    
private:
    size_t max_tokens_;
    
    // Helper methods
    std::vector<std::string> split_text(const std::string& text) const;
    bool is_punctuation(char c) const;
};

/**
 * @brief Tokenizer factory for creating appropriate tokenizers
 */
class TokenizerFactory {
public:
    enum class Type {
        BASIC,
        // Future: BPE, SENTENCEPIECE, etc.
    };
    
    static std::unique_ptr<Tokenizer> create(Type type, size_t max_tokens = 8192);
};

} // namespace chunking
} // namespace r3m 