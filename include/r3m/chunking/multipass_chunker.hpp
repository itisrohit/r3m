#pragma once

#include "r3m/chunking/tokenizer.hpp"
#include "r3m/chunking/sentence_chunker.hpp"
#include "r3m/chunking/chunk_models.hpp"
#include <memory>
#include <vector>

namespace r3m {
namespace chunking {

/**
 * @brief Multipass chunker for advanced document segmentation
 * 
 * Advanced multipass indexing system that provides:
 * - Mini-chunks for fine-grained detail retention
 * - Large chunks for broader context
 * - Dual embedding support
 */
class MultipassChunker {
public:
    /**
     * @brief Constructor with configuration
     * @param tokenizer Tokenizer for token counting
     * @param enable_multipass Enable multipass mode
     * @param enable_large_chunks Enable large chunk generation
     * @param mini_chunk_size Size for mini-chunks
     * @param large_chunk_ratio Number of regular chunks per large chunk
     * @param chunk_token_limit Target token limit for regular chunks
     */
    MultipassChunker(
        std::shared_ptr<Tokenizer> tokenizer,
        bool enable_multipass = false,
        bool enable_large_chunks = false,
        size_t mini_chunk_size = 150,
        size_t large_chunk_ratio = 4,
        size_t chunk_token_limit = 2048
    );
    
    ~MultipassChunker() = default;
    
    /**
     * @brief Chunk a document with multipass processing
     * @param document_id Document identifier
     * @param content Document content
     * @param title Document title
     * @param metadata Document metadata
     * @return ChunkingResult with all chunks
     */
    ChunkingResult chunk_document(
        const std::string& document_id,
        const std::string& content,
        const std::string& title = "",
        const std::unordered_map<std::string, std::string>& metadata = {}
    );
    
    /**
     * @brief Generate mini-chunks from regular chunks
     * @param chunks Regular chunks
     * @return Vector of mini-chunks
     */
    std::vector<DocumentChunk> generate_mini_chunks(const std::vector<DocumentChunk>& chunks);
    
    /**
     * @brief Generate large chunks from regular chunks
     * @param chunks Regular chunks
     * @return Vector of large chunks
     */
    std::vector<DocumentChunk> generate_large_chunks(const std::vector<DocumentChunk>& chunks);
    
    /**
     * @brief Get configuration
     */
    bool is_multipass_enabled() const { return enable_multipass_; }
    bool is_large_chunks_enabled() const { return enable_large_chunks_; }
    size_t get_mini_chunk_size() const { return mini_chunk_size_; }
    size_t get_large_chunk_ratio() const { return large_chunk_ratio_; }

private:
    std::shared_ptr<Tokenizer> tokenizer_;
    std::shared_ptr<SentenceChunker> regular_chunker_;
    std::shared_ptr<SentenceChunker> mini_chunker_;
    
    bool enable_multipass_;
    bool enable_large_chunks_;
    size_t mini_chunk_size_;
    size_t large_chunk_ratio_;
    
    // Helper methods
    std::string extract_blurb(const std::string& text) const;
    std::vector<std::string> get_mini_chunk_texts(const std::string& chunk_text) const;
    std::string create_metadata_string(const std::unordered_map<std::string, std::string>& metadata) const;
    std::string create_metadata_keyword(const std::unordered_map<std::string, std::string>& metadata) const;
    DocumentChunk create_chunk(
        int chunk_id,
        const std::string& document_id,
        const std::string& content,
        const std::string& title_prefix,
        const std::string& metadata_semantic,
        const std::string& metadata_keyword,
        bool is_continuation = false
    ) const;
};

} // namespace chunking
} // namespace r3m 