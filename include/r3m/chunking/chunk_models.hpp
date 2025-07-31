#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace r3m {
namespace chunking {

/**
 * @brief Base chunk structure containing core chunk information
 * 
 * BaseChunk, provides fundamental chunk properties.
 */
struct BaseChunk {
    int chunk_id;
    std::string blurb;           // First sentence(s) of the chunk
    std::string content;          // Main chunk text
    std::unordered_map<int, std::string> source_links;  // Links with offsets
    std::string image_file_id;    // Associated image file
    bool section_continuation;    // True if chunk doesn't start at section beginning
    
    BaseChunk() : chunk_id(0), section_continuation(false) {}
    
    BaseChunk(int id, const std::string& b, const std::string& c)
        : chunk_id(id), blurb(b), content(c), section_continuation(false) {}
};

/**
 * @brief Document-aware chunk with metadata and advanced features
 * 
 * Document-aware chunk with context and advanced features
 * like multipass support and contextual RAG.
 */
struct DocumentChunk : public BaseChunk {
    std::string document_id;      // Source document identifier
    std::string title_prefix;     // Document title prefix
    std::string metadata_semantic; // Semantic metadata string
    std::string metadata_keyword; // Keyword metadata string
    
    // Multipass support
    std::vector<std::string> mini_chunk_texts;  // For multipass mode
    int large_chunk_id;           // Reference to large chunk
    std::vector<int> large_chunk_reference_ids; // IDs of chunks in large chunk
    
    // Contextual RAG support
    int contextual_rag_reserved_tokens;
    std::string doc_summary;      // Document summary for RAG
    std::string chunk_context;    // Chunk-specific context
    
    // Quality metrics
    double quality_score;
    double information_density;
    bool is_high_quality;
    
    DocumentChunk() : large_chunk_id(-1), contextual_rag_reserved_tokens(0),
                     quality_score(0.0), information_density(0.0), is_high_quality(false) {}
    
    /**
     * @brief Get short descriptor for logging
     */
    std::string to_short_descriptor() const {
        return document_id + " Chunk ID: " + std::to_string(chunk_id);
    }
    
    /**
     * @brief Get primary link from source links
     */
    std::string get_primary_link() const {
        auto it = source_links.find(0);
        return (it != source_links.end()) ? it->second : "";
    }
};

/**
 * @brief Chunk with embedding information
 * 
 * Indexed chunk with embedding data for vector search.
 */
struct IndexedChunk : public DocumentChunk {
    std::vector<float> embedding;        // Main chunk embedding
    std::vector<float> title_embedding;  // Title embedding
    std::vector<std::vector<float>> mini_chunk_embeddings; // For multipass
    
    IndexedChunk() = default;
};

/**
 * @brief Chunk processing result
 * 
 * Contains the result of chunking a document with metadata.
 */
struct ChunkingResult {
    std::vector<DocumentChunk> chunks;
    size_t total_chunks;
    size_t successful_chunks;
    size_t failed_chunks;
    double processing_time_ms;
    
    // Quality metrics
    double avg_quality_score;
    double avg_information_density;
    size_t high_quality_chunks;
    
    ChunkingResult() : total_chunks(0), successful_chunks(0), failed_chunks(0),
                      processing_time_ms(0.0), avg_quality_score(0.0), 
                      avg_information_density(0.0), high_quality_chunks(0) {}
};

} // namespace chunking
} // namespace r3m 