#include "r3m/chunking/multipass_chunker.hpp"
#include <sstream>
#include <algorithm>
#include <unordered_set>

namespace r3m {
namespace chunking {

MultipassChunker::MultipassChunker(
    std::shared_ptr<Tokenizer> tokenizer,
    bool enable_multipass,
    bool enable_large_chunks,
    size_t mini_chunk_size,
    size_t large_chunk_ratio,
    size_t chunk_token_limit
) : tokenizer_(tokenizer),
    enable_multipass_(enable_multipass),
    enable_large_chunks_(enable_large_chunks),
    mini_chunk_size_(mini_chunk_size),
    large_chunk_ratio_(large_chunk_ratio),
    chunk_token_limit_(chunk_token_limit) {
    
    // Create regular chunker
    regular_chunker_ = std::make_shared<SentenceChunker>(
        tokenizer, chunk_token_limit, 0, "texts"
    );
    
    // Create mini-chunker if multipass is enabled
    if (enable_multipass_) {
        mini_chunker_ = std::make_shared<SentenceChunker>(
            tokenizer, mini_chunk_size, 0, "texts"
        );
    }
}

ChunkingResult MultipassChunker::chunk_document(
    const std::string& document_id,
    const std::string& content,
    const std::string& title,
    const std::unordered_map<std::string, std::string>& metadata
) {
    ChunkingResult result;
    
    if (content.empty()) {
        return result;
    }
    
    // Create metadata strings
    std::string metadata_semantic = create_metadata_string(metadata);
    std::string metadata_keyword = create_metadata_keyword(metadata);
    
    // Create title prefix
    std::string title_prefix = title.empty() ? "" : title + "\n";
    
    // Generate regular chunks
    auto text_chunks = regular_chunker_->chunk(content);
    std::vector<DocumentChunk> regular_chunks;
    
    for (size_t i = 0; i < text_chunks.size(); ++i) {
        auto chunk = create_chunk(
            i,
            document_id,
            text_chunks[i],
            title_prefix,
            metadata_semantic,
            metadata_keyword,
            i > 0
        );
        regular_chunks.push_back(chunk);
    }
    
    result.chunks = regular_chunks;
    result.total_chunks = regular_chunks.size();
    result.successful_chunks = regular_chunks.size();
    result.failed_chunks = 0;
    
    // Generate mini-chunks if multipass is enabled
    if (enable_multipass_) {
        auto mini_chunks = generate_mini_chunks(regular_chunks);
        result.chunks.insert(result.chunks.end(), mini_chunks.begin(), mini_chunks.end());
        result.total_chunks += mini_chunks.size();
        result.successful_chunks += mini_chunks.size();
    }
    
    // Generate large chunks if enabled
    if (enable_large_chunks_) {
        auto large_chunks = generate_large_chunks(regular_chunks);
        result.chunks.insert(result.chunks.end(), large_chunks.begin(), large_chunks.end());
        result.total_chunks += large_chunks.size();
        result.successful_chunks += large_chunks.size();
    }
    
    // Calculate quality metrics
    double total_quality = 0.0;
    double total_density = 0.0;
    size_t high_quality_count = 0;
    
    for (const auto& chunk : result.chunks) {
        total_quality += chunk.quality_score;
        total_density += chunk.information_density;
        if (chunk.is_high_quality) {
            high_quality_count++;
        }
    }
    
    if (!result.chunks.empty()) {
        result.avg_quality_score = total_quality / result.chunks.size();
        result.avg_information_density = total_density / result.chunks.size();
        result.high_quality_chunks = high_quality_count;
    }
    
    return result;
}

std::vector<DocumentChunk> MultipassChunker::generate_mini_chunks(
    const std::vector<DocumentChunk>& chunks
) {
    std::vector<DocumentChunk> mini_chunks;
    
    if (!mini_chunker_) {
        return mini_chunks;
    }
    
    for (const auto& chunk : chunks) {
        auto mini_texts = get_mini_chunk_texts(chunk.content);
        
        for (size_t i = 0; i < mini_texts.size(); ++i) {
            auto mini_chunk = create_chunk(
                mini_chunks.size(),
                chunk.document_id,
                mini_texts[i],
                chunk.title_prefix,
                chunk.metadata_semantic,
                chunk.metadata_keyword,
                i > 0
            );
            
            // Set mini-chunk specific properties
            mini_chunk.mini_chunk_texts = mini_texts;
            mini_chunk.large_chunk_id = chunk.chunk_id;
            mini_chunk.large_chunk_reference_ids = {chunk.chunk_id};
            
            mini_chunks.push_back(mini_chunk);
        }
    }
    
    return mini_chunks;
}

std::vector<DocumentChunk> MultipassChunker::generate_large_chunks(
    const std::vector<DocumentChunk>& chunks
) {
    std::vector<DocumentChunk> large_chunks;
    
    if (chunks.empty()) {
        return large_chunks;
    }
    
    // Group chunks into large chunks based on ratio
    for (size_t i = 0; i < chunks.size(); i += large_chunk_ratio_) {
        size_t end_idx = std::min(i + large_chunk_ratio_, chunks.size());
        
        // Combine content from multiple chunks
        std::string combined_content;
        std::vector<int> reference_ids;
        
        for (size_t j = i; j < end_idx; ++j) {
            if (!combined_content.empty()) {
                combined_content += "\n\n";
            }
            combined_content += chunks[j].content;
            reference_ids.push_back(chunks[j].chunk_id);
        }
        
        // Create large chunk
        auto large_chunk = create_chunk(
            large_chunks.size(),
            chunks[i].document_id,
            combined_content,
            chunks[i].title_prefix,
            chunks[i].metadata_semantic,
            chunks[i].metadata_keyword,
            false
        );
        
        // Set large chunk specific properties
        large_chunk.large_chunk_id = large_chunks.size();
        large_chunk.large_chunk_reference_ids = reference_ids;
        
        large_chunks.push_back(large_chunk);
    }
    
    return large_chunks;
}

std::string MultipassChunker::extract_blurb(const std::string& text) const {
    if (text.empty()) {
        return "";
    }
    
    // Extract first sentence or first 100 characters
    size_t first_period = text.find('.');
    size_t first_exclamation = text.find('!');
    size_t first_question = text.find('?');
    
    size_t end_pos = text.length();
    if (first_period != std::string::npos) {
        end_pos = std::min(end_pos, first_period + 1);
    }
    if (first_exclamation != std::string::npos) {
        end_pos = std::min(end_pos, first_exclamation + 1);
    }
    if (first_question != std::string::npos) {
        end_pos = std::min(end_pos, first_question + 1);
    }
    
    std::string blurb = text.substr(0, std::min(end_pos, size_t(100)));
    
    // Clean up the blurb
    while (!blurb.empty() && std::isspace(blurb.back())) {
        blurb.pop_back();
    }
    
    return blurb;
}

std::vector<std::string> MultipassChunker::get_mini_chunk_texts(const std::string& chunk_text) const {
    if (!mini_chunker_ || chunk_text.empty()) {
        return {};
    }
    
    return mini_chunker_->chunk(chunk_text);
}

std::string MultipassChunker::create_metadata_string(
    const std::unordered_map<std::string, std::string>& metadata
) const {
    if (metadata.empty()) {
        return "";
    }
    
    std::stringstream ss;
    ss << "Metadata:\n";
    
    for (const auto& [key, value] : metadata) {
        ss << "\t" << key << " - " << value << "\n";
    }
    
    std::string result = ss.str();
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    
    return result;
}

std::string MultipassChunker::create_metadata_keyword(
    const std::unordered_map<std::string, std::string>& metadata
) const {
    if (metadata.empty()) {
        return "";
    }
    
    std::vector<std::string> values;
    for (const auto& [key, value] : metadata) {
        values.push_back(value);
    }
    
    std::string result;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            result += " ";
        }
        result += values[i];
    }
    
    return result;
}

DocumentChunk MultipassChunker::create_chunk(
    int chunk_id,
    const std::string& document_id,
    const std::string& content,
    const std::string& title_prefix,
    const std::string& metadata_semantic,
    const std::string& metadata_keyword,
    bool is_continuation
) const {
    DocumentChunk chunk;
    
    chunk.chunk_id = chunk_id;
    chunk.document_id = document_id;
    chunk.blurb = extract_blurb(content);
    chunk.content = content;
    chunk.title_prefix = title_prefix;
    chunk.metadata_semantic = metadata_semantic;
    chunk.metadata_keyword = metadata_keyword;
    chunk.section_continuation = is_continuation;
    
    // Calculate basic quality metrics
    size_t word_count = 0;
    size_t unique_words = 0;
    std::unordered_set<std::string> words;
    
    std::stringstream ss(content);
    std::string word;
    while (ss >> word) {
        word_count++;
        words.insert(word);
    }
    
    unique_words = words.size();
    
    // Simple quality scoring based on content length and word diversity
    if (word_count > 0) {
        chunk.quality_score = std::min(1.0, (double)word_count / 100.0);
        chunk.information_density = std::min(1.0, (double)unique_words / (double)word_count);
        chunk.is_high_quality = chunk.quality_score > 0.5 && chunk.information_density > 0.3;
    } else {
        chunk.quality_score = 0.0;
        chunk.information_density = 0.0;
        chunk.is_high_quality = false;
    }
    
    return chunk;
}

} // namespace chunking
} // namespace r3m 