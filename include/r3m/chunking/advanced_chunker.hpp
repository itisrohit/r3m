#pragma once

#include "r3m/chunking/tokenizer.hpp"
#include "r3m/chunking/multipass_chunker.hpp"
#include "r3m/chunking/contextual_rag.hpp"
#include "r3m/chunking/chunk_models.hpp"
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace r3m {
namespace chunking {

/**
 * @brief Advanced chunking manager with all advanced features
 * 
 * Integrates multipass indexing, contextual RAG, and quality filtering
 * Comprehensive chunking system with advanced features
 */
class AdvancedChunker {
public:
    /**
     * @brief Configuration for advanced chunking
     */
    struct Config {
        // Tokenizer settings
        size_t max_tokens = 8192;
        size_t chunk_token_limit = 2048;
        size_t chunk_overlap = 0;
        
        // Chunk sizes
        size_t blurb_size = 100;
        size_t mini_chunk_size = 150;
        size_t large_chunk_ratio = 4;
        size_t chunk_min_content = 256;
        
        // Metadata settings
        bool include_metadata = true;
        double max_metadata_percentage = 0.25;
        bool skip_metadata_in_chunk = false;
        
        // Advanced features
        bool enable_multipass = false;
        bool enable_large_chunks = false;
        bool enable_contextual_rag = false;
        
        // Contextual RAG settings
        size_t reserved_tokens = 512;
        bool use_document_summary = true;
        bool use_chunk_summary = true;
        
        // Quality filtering
        bool enable_quality_filtering = true;
        double min_chunk_quality = 0.3;
        double min_chunk_density = 0.1;
        size_t min_chunk_length = 50;
        size_t max_chunk_length = 10000;
        
        // Processing settings
        bool enable_parallel_chunking = true;
        size_t chunk_worker_threads = 4;
        size_t batch_chunk_size = 16;
    };
    
    /**
     * @brief Constructor with configuration
     * @param config Advanced chunking configuration
     */
    explicit AdvancedChunker(const Config& config);
    
    ~AdvancedChunker() = default;
    
    /**
     * @brief Chunk a document with all advanced features
     * @param document_id Document identifier
     * @param content Document content
     * @param title Document title
     * @param metadata Document metadata
     * @return ChunkingResult with all chunks and metrics
     */
    ChunkingResult chunk_document(
        const std::string& document_id,
        const std::string& content,
        const std::string& title = "",
        const std::unordered_map<std::string, std::string>& metadata = {}
    );
    
    /**
     * @brief Chunk multiple documents in batch
     * @param documents Vector of document information
     * @return Vector of chunking results
     */
    std::vector<ChunkingResult> chunk_documents_batch(
        const std::vector<std::tuple<std::string, std::string, std::string, std::unordered_map<std::string, std::string>>>& documents
    );
    
    /**
     * @brief Filter chunks based on quality criteria
     * @param chunks Vector of chunks to filter
     * @return Filtered chunks
     */
    std::vector<DocumentChunk> filter_chunks_by_quality(const std::vector<DocumentChunk>& chunks);
    
    /**
     * @brief Get configuration
     */
    const Config& get_config() const { return config_; }
    
    /**
     * @brief Update configuration
     */
    void update_config(const Config& config);

private:
    Config config_;
    std::shared_ptr<Tokenizer> tokenizer_;
    std::unique_ptr<MultipassChunker> multipass_chunker_;
    std::unique_ptr<ContextualRAG> contextual_rag_;
    
    // Helper methods
    void initialize_components();
    std::vector<DocumentChunk> apply_quality_filtering(const std::vector<DocumentChunk>& chunks);
    bool should_include_chunk(const DocumentChunk& chunk) const;
    void calculate_chunk_metrics(DocumentChunk& chunk) const;
};

} // namespace chunking
} // namespace r3m 