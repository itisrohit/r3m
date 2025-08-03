#pragma once

#include "r3m/chunking/chunk_models.hpp"
#include "r3m/chunking/tokenizer.hpp"
#include <memory>
#include <vector>
#include <string>

namespace r3m {
namespace chunking {

/**
 * @brief Contextual RAG system for enhanced chunk context
 * 
 * Advanced contextual RAG system that provides:
 * - Document-level summaries
 * - Chunk-specific context generation
 * - Token reservation for contextual information
 */
class ContextualRAG {
public:
    /**
     * @brief Constructor with configuration
     * @param tokenizer Tokenizer for token counting
     * @param reserved_tokens Tokens reserved for contextual information
     * @param use_document_summary Enable document-level summaries
     * @param use_chunk_summary Enable chunk-level summaries
     */
    ContextualRAG(
        std::shared_ptr<Tokenizer> tokenizer,
        size_t reserved_tokens = 512,
        bool use_document_summary = true,
        bool use_chunk_summary = true
    );
    
    ~ContextualRAG() = default;
    
    /**
     * @brief Add contextual information to chunks
     * @param chunks Vector of chunks to enhance
     * @return Enhanced chunks with contextual information
     */
    std::vector<DocumentChunk> add_contextual_summaries(
        std::vector<DocumentChunk>& chunks
    );
    
    /**
     * @brief Generate document summary
     * @param chunks All chunks from a document
     * @return Document summary string
     */
    std::string generate_document_summary(const std::vector<DocumentChunk>& chunks);
    
    /**
     * @brief Generate chunk context
     * @param chunk The chunk to generate context for
     * @param document_summary Document summary
     * @return Chunk context string
     */
    std::string generate_chunk_context(
        const DocumentChunk& chunk,
        const std::string& document_summary
    );
    
    /**
     * @brief Check if document fits in single chunk
     * @param chunks All chunks from a document
     * @return True if document fits in single chunk
     */
    bool document_fits_in_single_chunk(const std::vector<DocumentChunk>& chunks);
    
    /**
     * @brief Get configuration
     */
    bool is_document_summary_enabled() const { return use_document_summary_; }
    bool is_chunk_summary_enabled() const { return use_chunk_summary_; }
    size_t get_reserved_tokens() const { return reserved_tokens_; }

private:
    std::shared_ptr<Tokenizer> tokenizer_;
    size_t reserved_tokens_;
    bool use_document_summary_;
    bool use_chunk_summary_;
    
    // Helper methods
    std::string create_document_summary_prompt(const std::vector<DocumentChunk>& chunks);
    std::string create_chunk_context_prompt(const DocumentChunk& chunk, const std::string& document_summary);
    std::string simulate_llm_response(const std::string& prompt);
    size_t count_tokens(const std::string& text) const;
};

} // namespace chunking
} // namespace r3m 