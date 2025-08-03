#include "r3m/chunking/token_management/token_cache.hpp"

namespace r3m {
namespace chunking {
namespace token_management {

// OptimizedTokenCache implementation
OptimizedTokenCache::OptimizedTokenCache(std::shared_ptr<Tokenizer> tokenizer) : tokenizer_(tokenizer) {}

int OptimizedTokenCache::get_token_count(std::string_view text) {
    auto it = cache_.find(text);
    if (it != cache_.end()) {
        return it->second;
    }
    
    // Store the string to keep the view valid
    string_storage_.emplace_back(text);
    std::string_view stored_view = string_storage_.back();
    
    int count = static_cast<int>(tokenizer_->count_tokens(std::string(stored_view)));
    cache_[stored_view] = count;
    return count;
}

void OptimizedTokenCache::clear() {
    cache_.clear();
    string_storage_.clear();
}

// TokenCache implementation (backward compatibility)
TokenCache::TokenCache(std::shared_ptr<Tokenizer> tokenizer) : tokenizer_(tokenizer) {}

int TokenCache::get_token_count(const std::string& text) {
    auto it = cache_.find(text);
    if (it != cache_.end()) {
        return it->second;
    }
    
    int count = static_cast<int>(tokenizer_->count_tokens(text));
    cache_[text] = count;
    return count;
}

void TokenCache::clear() {
    cache_.clear();
}

} // namespace token_management
} // namespace chunking
} // namespace r3m 