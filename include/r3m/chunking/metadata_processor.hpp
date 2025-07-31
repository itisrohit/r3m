#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace r3m {
namespace chunking {

/**
 * @brief Advanced metadata processor for document chunking
 * 
 * Implements sophisticated metadata processing,
 * including semantic and keyword metadata generation with proper formatting.
 */
class MetadataProcessor {
public:
    /**
     * @brief Metadata processing result
     */
    struct MetadataResult {
        std::string semantic_suffix;    // Semantic metadata string
        std::string keyword_suffix;     // Keyword metadata string
        int semantic_tokens;            // Token count for semantic metadata
        int keyword_tokens;             // Token count for keyword metadata
        bool is_valid;                  // Whether metadata is valid
        
        MetadataResult() : semantic_tokens(0), keyword_tokens(0), is_valid(false) {}
    };
    
    /**
     * @brief Process document metadata into semantic and keyword strings
     * @param metadata Document metadata map
     * @param include_separator Whether to include separator in output
     * @return Processed metadata result
     */
    static MetadataResult process_metadata(
        const std::unordered_map<std::string, std::string>& metadata,
        bool include_separator = false
    );
    
    /**
     * @brief Get metadata suffix for document indexing
     * @param metadata Document metadata map
     * @param include_separator Whether to include separator
     * @return Tuple of (semantic_suffix, keyword_suffix)
     */
    static std::pair<std::string, std::string> get_metadata_suffix_for_document_index(
        const std::unordered_map<std::string, std::string>& metadata,
        bool include_separator = false
    );
    
    /**
     * @brief Check if metadata is too large for chunk
     * @param metadata_tokens Number of metadata tokens
     * @param chunk_token_limit Total chunk token limit
     * @return True if metadata should be skipped
     */
    static bool is_metadata_too_large(int metadata_tokens, int chunk_token_limit);
    
    /**
     * @brief Clean and format metadata value
     * @param value Raw metadata value
     * @return Cleaned metadata value
     */
    static std::string clean_metadata_value(const std::string& value);
    
    /**
     * @brief Extract semantic metadata (natural language format)
     * @param metadata Document metadata
     * @return Semantic metadata string
     */
    static std::string extract_semantic_metadata(
        const std::unordered_map<std::string, std::string>& metadata
    );
    
    /**
     * @brief Extract keyword metadata (structured format)
     * @param metadata Document metadata
     * @return Keyword metadata string
     */
    static std::string extract_keyword_metadata(
        const std::unordered_map<std::string, std::string>& metadata
    );
    
    /**
     * @brief Extract all metadata  (natural language)
     * @param metadata Document metadata
     * @return All metadata as natural language string
     */
    static std::string extract_all_metadata(
        const std::unordered_map<std::string, std::string>& metadata
    );
    
    /**
     * @brief Extract all metadata values for keyword search
     * @param metadata Document metadata
     * @return All metadata values as space-separated string
     */
    static std::string extract_all_values(
        const std::unordered_map<std::string, std::string>& metadata
    );
    
private:
    // Metadata keys to ignore
    static const std::vector<std::string> IGNORED_METADATA_KEYS;
    
    // Metadata keys that should be treated as semantic
    static const std::vector<std::string> SEMANTIC_METADATA_KEYS;
    
    // Metadata keys that should be treated as keywords
    static const std::vector<std::string> KEYWORD_METADATA_KEYS;
    
    /**
     * @brief Check if metadata key should be ignored
     * @param key Metadata key
     * @return True if key should be ignored
     */
    static bool should_ignore_metadata_key(const std::string& key);
    
    /**
     * @brief Check if metadata key is semantic
     * @param key Metadata key
     * @return True if key is semantic
     */
    static bool is_semantic_metadata_key(const std::string& key);
    
    /**
     * @brief Check if metadata key is keyword
     * @param key Metadata key
     * @return True if key is keyword
     */
    static bool is_keyword_metadata_key(const std::string& key);
};

} // namespace chunking
} // namespace r3m 