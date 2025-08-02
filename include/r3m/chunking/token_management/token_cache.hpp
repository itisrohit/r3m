#pragma once

#include "r3m/chunking/tokenizer.hpp"
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <memory>

namespace r3m {
namespace chunking {
namespace token_management {

/**
 * @brief Optimized token cache with string_view for high performance
 * 
 * Provides efficient token counting with caching using string_view
 * for minimal memory overhead and maximum performance.
 */
class OptimizedTokenCache {
private:
    std::unordered_map<std::string_view, int> cache_;
    std::shared_ptr<Tokenizer> tokenizer_;
    std::vector<std::string> string_storage_; // Keep strings alive
    
public:
    /**
     * @brief Constructor
     * @param tokenizer Shared pointer to tokenizer
     */
    explicit OptimizedTokenCache(std::shared_ptr<Tokenizer> tokenizer);
    
    /**
     * @brief Get token count for text with caching
     * @param text Text to count tokens for
     * @return Number of tokens
     */
    int get_token_count(std::string_view text);
    
    /**
     * @brief Clear the cache
     */
    void clear();
};

/**
 * @brief Traditional token cache for backward compatibility
 * 
 * Provides token counting with caching using std::string
 * for compatibility with existing code.
 */
class TokenCache {
private:
    std::unordered_map<std::string, int> cache_;
    std::shared_ptr<Tokenizer> tokenizer_;
    
public:
    /**
     * @brief Constructor
     * @param tokenizer Shared pointer to tokenizer
     */
    explicit TokenCache(std::shared_ptr<Tokenizer> tokenizer);
    
    /**
     * @brief Get token count for text with caching
     * @param text Text to count tokens for
     * @return Number of tokens
     */
    int get_token_count(const std::string& text);
    
    /**
     * @brief Clear the cache
     */
    void clear();
};

} // namespace token_management
} // namespace chunking
} // namespace r3m 