#pragma once

#include "r3m/chunking/chunk_models.hpp"
#include "r3m/chunking/tokenizer.hpp"
#include "r3m/chunking/metadata_processor.hpp"
#include "r3m/chunking/sentence_chunker.hpp"
#include "r3m/chunking/multipass_chunker.hpp"
#include "r3m/chunking/contextual_rag.hpp"
#include "r3m/chunking/quality_assessment/quality_calculator.hpp"
#include "r3m/chunking/token_management/token_cache.hpp"
#include "r3m/chunking/section_processing/section_processor.hpp"
#include <memory>
#include <unordered_map>
#include <string>
#include <string_view> // Added for string_view
#include <vector> // Added for std::vector

namespace r3m {
namespace chunking {

// Forward declarations
struct DocumentInfo;



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
     * @brief Document information
     */
    struct DocumentInfo {
        std::string document_id;
        std::string title;
        std::string semantic_identifier;
        std::string source_type;
        std::unordered_map<std::string, std::string> metadata;
        std::vector<section_processing::DocumentSection> sections;
        std::string full_content;
        int total_tokens = 0;
        
        DocumentInfo() = default;
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
    std::unique_ptr<token_management::TokenCache> token_cache_; // Token cache for performance optimization
    std::unique_ptr<token_management::OptimizedTokenCache> optimized_cache_; // Optimized token cache
    std::unique_ptr<section_processing::SectionProcessor> section_processor_; // Section processor
    
    /**
     * @brief Manage token allocation for title, metadata, and content
     * @param document Document information
     * @return Token management result
     */
    section_processing::TokenManagementResult manage_tokens(const DocumentInfo& document);
    
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
        const section_processing::TokenManagementResult& token_result
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