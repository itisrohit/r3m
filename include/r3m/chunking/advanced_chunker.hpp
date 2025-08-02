#pragma once

#include "r3m/chunking/chunk_models.hpp"
#include "r3m/chunking/tokenizer.hpp"
#include "r3m/chunking/metadata_processor.hpp"
#include "r3m/chunking/sentence_chunker.hpp"
#include "r3m/chunking/multipass_chunker.hpp"
#include "r3m/chunking/contextual_rag.hpp"
#include "r3m/chunking/quality_assessment/quality_calculator.hpp"
#include <memory>
#include <unordered_map>
#include <string>
#include <string_view> // Added for string_view
#include <vector> // Added for std::vector

namespace r3m {
namespace chunking {

// Forward declarations
struct DocumentSection;
struct DocumentInfo;

// Optimized token cache with string_view
class OptimizedTokenCache {
private:
    std::unordered_map<std::string_view, int> cache_;
    std::shared_ptr<Tokenizer> tokenizer_;
    std::vector<std::string> string_storage_; // Keep strings alive
    
public:
    OptimizedTokenCache(std::shared_ptr<Tokenizer> tokenizer);
    int get_token_count(std::string_view text);
    void clear();
};

// Token cache for performance optimization
class TokenCache {
private:
    std::unordered_map<std::string, int> cache_;
    std::shared_ptr<Tokenizer> tokenizer_;
    
public:
    TokenCache(std::shared_ptr<Tokenizer> tokenizer);
    int get_token_count(const std::string& text);
    void clear();
};

/**
 * @brief Advanced chunker with sophisticated token management
 * 
 * Comprehensive chunking system with advanced features,
 * including precise token management, metadata processing, and section handling.
 */
class AdvancedChunker {
public:
    /**
     * @brief Chunker configuration
     */
    struct Config {
        bool enable_multipass = false;
        bool enable_large_chunks = false;
        bool enable_contextual_rag = false;
        bool include_metadata = true;
        int chunk_token_limit = 2048;
        int chunk_overlap = 0;
        int mini_chunk_size = 150;
        int blurb_size = 100;
        int large_chunk_ratio = 4;
        int chunk_min_content = 256;
        double max_metadata_percentage = 0.25;
        int contextual_rag_reserved_tokens = 512;
        
        Config() {}
    };
    
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
     * @brief Document information
     */
    struct DocumentInfo {
        std::string document_id;
        std::string title;
        std::string semantic_identifier;
        std::string source_type;
        std::unordered_map<std::string, std::string> metadata;
        std::vector<DocumentSection> sections;
        std::string full_content;
        int total_tokens = 0;
        
        DocumentInfo() = default;
    };
    
    /**
     * @brief Token management result
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
    
    explicit AdvancedChunker(std::shared_ptr<Tokenizer> tokenizer, const Config& config = Config());
    ~AdvancedChunker() = default;
    
    /**
     * @brief Process a single document
     */
    ChunkingResult process_document(const DocumentInfo& document);
    
    /**
     * @brief Process multiple documents
     */
    std::vector<ChunkingResult> process_documents(const std::vector<DocumentInfo>& documents);
    
    /**
     * @brief Get chunker configuration
     * @return Current configuration
     */
    const Config& get_config() const { return config_; }
    
    /**
     * @brief Update chunker configuration
     */
    void update_config(const Config& config);
    
    /**
     * @brief Clear internal caches for memory management
     */
    void clear_cache();
    
private:
    std::shared_ptr<Tokenizer> tokenizer_;
    Config config_;
    std::unique_ptr<SentenceChunker> blurb_splitter_;
    std::unique_ptr<SentenceChunker> chunk_splitter_;
    std::unique_ptr<SentenceChunker> mini_chunk_splitter_;
    std::unique_ptr<MultipassChunker> multipass_chunker_;
    std::unique_ptr<ContextualRAG> contextual_rag_;
    std::unique_ptr<TokenCache> token_cache_; // Forward declaration for performance optimization
    std::unique_ptr<OptimizedTokenCache> optimized_cache_; // Optimized token cache
    
    /**
     * @brief Manage token allocation for title, metadata, and content
     * @param document Document information
     * @return Token management result
     */
    TokenManagementResult manage_tokens(const DocumentInfo& document);
    
    /**
     * @brief Extract title blurb from document
     * @param title Document title
     * @return Title blurb
     */
    std::string extract_title_blurb(const std::string& title);
    
    /**
     * @brief Process document sections with token management
     * @param document Document information
     * @param token_result Token management result
     * @return Vector of document chunks
     */
    std::vector<DocumentChunk> process_sections(
        const DocumentInfo& document,
        const TokenManagementResult& token_result
    );
    
    /**
     * @brief Process sections with advanced section combination logic
     */
    std::vector<DocumentChunk> process_sections_with_combinations(
        const DocumentInfo& document,
        const TokenManagementResult& token_result
    );
    
    /**
     * @brief Split oversized sections
     * @param sections Document sections
     * @param content_token_limit Content token limit
     * @return Processed sections
     */
    std::vector<DocumentSection> split_oversized_sections(
        const std::vector<DocumentSection>& sections,
        int content_token_limit
    );

    /**
     * @brief Split oversized chunk into smaller chunks
     * @param text Text to split
     * @param content_token_limit Token limit for each chunk
     * @return Vector of smaller chunks
     */
    std::vector<std::string> split_oversized_chunk(
        const std::string& text,
        int content_token_limit
    );
    
    std::vector<std::string> split_oversized_chunk_optimized(
        const std::string& text,
        int content_token_limit
    );

    /**
     * @brief Handle image sections (force separate chunks)
     * @param sections Document sections
     * @param content_token_limit Content token limit
     * @return Processed sections with image handling
     */
    std::vector<DocumentSection> handle_image_sections(
        const std::vector<DocumentSection>& sections,
        int content_token_limit
    );

    /**
     * @brief Create chunk from section content
     * @param section Document section
     * @param chunk_id Chunk ID
     * @param document_id Document ID
     * @param title_prefix Title prefix
     * @param metadata_suffix_semantic Semantic metadata suffix
     * @param metadata_suffix_keyword Keyword metadata suffix
     * @param content_token_limit Content token limit
     * @param source_type Document source type
     * @param semantic_identifier Document semantic identifier
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
        bool is_continuation = false
    );
    
    /**
     * @brief Apply quality filtering to chunks
     * @param chunks Vector of chunks
     * @return Filtered chunks
     */
    std::vector<DocumentChunk> apply_quality_filtering(const std::vector<DocumentChunk>& chunks);
    
    /**
     * @brief Calculate chunk quality metrics
     * @param chunk Document chunk
     */
    void calculate_chunk_quality(DocumentChunk& chunk);
    
         /**
      * @brief Initialize chunking components
      */
     void initialize_components();
};

} // namespace chunking
} // namespace r3m 