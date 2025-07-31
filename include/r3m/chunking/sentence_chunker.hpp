#pragma once

#include "r3m/chunking/tokenizer.hpp"
#include <string>
#include <vector>
#include <memory>

namespace r3m {
namespace chunking {

/**
 * @brief Sentence-aware chunker for semantic text segmentation
 * 
 * Intelligent text chunking with sentence boundary detection
 * that respects sentence boundaries and token limits.
 */
class SentenceChunker {
public:
    /**
     * @brief Constructor with tokenizer and configuration
     * @param tokenizer Tokenizer for token counting
     * @param chunk_size Target chunk size in tokens
     * @param chunk_overlap Token overlap between chunks
     * @param return_type Type of return ("texts" for text chunks)
     */
    SentenceChunker(
        std::shared_ptr<Tokenizer> tokenizer,
        size_t chunk_size,
        size_t chunk_overlap = 0,
        const std::string& return_type = "texts"
    );
    
    ~SentenceChunker() = default;
    
    /**
     * @brief Chunk text into smaller pieces
     * @param text Input text to chunk
     * @return Vector of text chunks
     */
    std::vector<std::string> chunk(const std::string& text) const;
    
    /**
     * @brief Get chunk size
     */
    size_t get_chunk_size() const { return chunk_size_; }
    
    /**
     * @brief Get chunk overlap
     */
    size_t get_chunk_overlap() const { return chunk_overlap_; }

private:
    std::shared_ptr<Tokenizer> tokenizer_;
    size_t chunk_size_;
    size_t chunk_overlap_;
    std::string return_type_;
    
    // Helper methods
    std::vector<std::string> split_into_sentences(const std::string& text) const;
    std::vector<std::string> merge_sentences_into_chunks(
        const std::vector<std::string>& sentences
    ) const;
    bool should_start_new_chunk(
        const std::string& current_chunk,
        const std::string& next_sentence
    ) const;
    std::string clean_sentence(const std::string& sentence) const;
};

} // namespace chunking
} // namespace r3m 