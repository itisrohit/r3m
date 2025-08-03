#pragma once

#include "r3m/chunking/chunk_models.hpp"
#include "r3m/chunking/tokenizer.hpp"
#include "r3m/chunking/token_management/token_cache.hpp"
#include "r3m/chunking/sentence_chunker.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace r3m {
namespace chunking {
namespace section_processing {

/**
 * @brief Document section information
 */
struct DocumentSection {
    std::string content;
    std::string link;
    bool is_image = false;
    std::string image_file_id;
    bool is_oversized = false;
    int token_count = 0;
    
    DocumentSection() = default;
    DocumentSection(const std::string& c, const std::string& l = "")
        : content(c), link(l) {}
};

/**
 * @brief Token management result for section processing
 */
struct TokenManagementResult {
    std::string title_prefix;
    std::string metadata_suffix_semantic;
    std::string metadata_suffix_keyword;
    int title_tokens = 0;
    int metadata_tokens = 0;
    int content_token_limit = 0;
    int contextual_rag_reserved_tokens = 0;
    bool single_chunk_fits = true;
    bool metadata_too_large = false;
    
    TokenManagementResult() = default;
};

/**
 * @brief Advanced section processor with sophisticated combination logic
 * 
 * Handles complex section processing including:
 * - Section combination and splitting
 * - Image section handling
 * - Oversized section management
 * - Optimized token counting
 */
class SectionProcessor {
public:
    /**
     * @brief Constructor
     * @param tokenizer Shared pointer to tokenizer
     */
    explicit SectionProcessor(
        std::shared_ptr<Tokenizer> tokenizer
    );
    
    /**
     * @brief Process document sections with advanced combination logic
     * @param sections Document sections to process
     * @param token_result Token management result
     * @param document_id Document identifier
     * @param source_type Source type
     * @param semantic_identifier Semantic identifier
     * @return Vector of document chunks
     */
    std::vector<DocumentChunk> process_sections_with_combinations(
        const std::vector<DocumentSection>& sections,
        const TokenManagementResult& token_result,
        const std::string& document_id,
        const std::string& source_type,
        const std::string& semantic_identifier
    );
    
    /**
     * @brief Split oversized sections
     * @param sections Document sections
     * @param content_token_limit Content token limit
     * @return Vector of split sections
     */
    std::vector<DocumentSection> split_oversized_sections(
        const std::vector<DocumentSection>& sections,
        int content_token_limit
    );
    
    /**
     * @brief Split oversized chunk using sentence chunker
     * @param text Text to split
     * @param content_token_limit Content token limit
     * @return Vector of split texts
     */
    std::vector<std::string> split_oversized_chunk(
        const std::string& text,
        int content_token_limit
    );
    
    /**
     * @brief Split oversized chunk using optimized approach
     * @param text Text to split
     * @param content_token_limit Content token limit
     * @return Vector of split texts
     */
    std::vector<std::string> split_oversized_chunk_optimized(
        const std::string& text,
        int content_token_limit
    );
    
    /**
     * @brief Handle image sections
     * @param sections Document sections
     * @param content_token_limit Content token limit
     * @return Vector of processed sections
     */
    std::vector<DocumentSection> handle_image_sections(
        const std::vector<DocumentSection>& sections,
        int content_token_limit
    );
    
    /**
     * @brief Create chunk from section
     * @param section Document section
     * @param chunk_id Chunk identifier
     * @param document_id Document identifier
     * @param title_prefix Title prefix
     * @param metadata_suffix_semantic Semantic metadata suffix
     * @param metadata_suffix_keyword Keyword metadata suffix
     * @param content_token_limit Content token limit
     * @param source_type Source type
     * @param semantic_identifier Semantic identifier
     * @param is_continuation Whether this is a continuation chunk
     * @return Document chunk
     */
    DocumentChunk create_chunk_from_section(
        const DocumentSection& section,
        int chunk_id,
        const std::string& document_id,
        const std::string& title_prefix,
        const std::string& metadata_suffix_semantic,
        const std::string& metadata_suffix_keyword,
        int content_token_limit,
        const std::string& source_type,
        const std::string& semantic_identifier,
        bool is_continuation
    );

private:
    std::shared_ptr<Tokenizer> tokenizer_;
    std::unique_ptr<token_management::OptimizedTokenCache> optimized_cache_;
    std::unique_ptr<SentenceChunker> chunk_splitter_;
};

} // namespace section_processing
} // namespace chunking
} // namespace r3m 