#include "r3m/chunking/advanced_chunker.hpp"
#include <chrono>
#include <algorithm>
#include <unordered_set>

namespace r3m {
namespace chunking {

AdvancedChunker::AdvancedChunker(const Config& config)
    : config_(config) {
    initialize_components();
}

void AdvancedChunker::initialize_components() {
    // Initialize tokenizer
    tokenizer_ = TokenizerFactory::create(TokenizerFactory::Type::BASIC, config_.max_tokens);
    
    // Initialize multipass chunker
    multipass_chunker_ = std::make_unique<MultipassChunker>(
        tokenizer_,
        config_.enable_multipass,
        config_.enable_large_chunks,
        config_.mini_chunk_size,
        config_.large_chunk_ratio,
        config_.chunk_token_limit
    );
    
    // Initialize contextual RAG
    contextual_rag_ = std::make_unique<ContextualRAG>(
        tokenizer_,
        config_.reserved_tokens,
        config_.use_document_summary,
        config_.use_chunk_summary
    );
}

ChunkingResult AdvancedChunker::chunk_document(
    const std::string& document_id,
    const std::string& content,
    const std::string& title,
    const std::unordered_map<std::string, std::string>& metadata
) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    ChunkingResult result;
    
    if (content.empty()) {
        return result;
    }
    
    // Step 1: Generate chunks using multipass chunker
    auto multipass_result = multipass_chunker_->chunk_document(
        document_id, content, title, metadata
    );
    
    // Step 2: Apply quality filtering if enabled
    std::vector<DocumentChunk> filtered_chunks;
    if (config_.enable_quality_filtering) {
        filtered_chunks = apply_quality_filtering(multipass_result.chunks);
    } else {
        filtered_chunks = multipass_result.chunks;
    }
    
    // Step 3: Add contextual RAG if enabled
    if (config_.enable_contextual_rag) {
        contextual_rag_->add_contextual_summaries(filtered_chunks);
    }
    
    // Step 4: Calculate final metrics
    result.chunks = filtered_chunks;
    result.total_chunks = multipass_result.total_chunks;
    result.successful_chunks = filtered_chunks.size();
    result.failed_chunks = multipass_result.total_chunks - filtered_chunks.size();
    
    // Calculate quality metrics
    double total_quality = 0.0;
    double total_density = 0.0;
    size_t high_quality_count = 0;
    
    for (const auto& chunk : filtered_chunks) {
        total_quality += chunk.quality_score;
        total_density += chunk.information_density;
        if (chunk.is_high_quality) {
            high_quality_count++;
        }
    }
    
    if (!filtered_chunks.empty()) {
        result.avg_quality_score = total_quality / filtered_chunks.size();
        result.avg_information_density = total_density / filtered_chunks.size();
        result.high_quality_chunks = high_quality_count;
    }
    
    // Calculate processing time
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    result.processing_time_ms = duration.count() / 1000.0;
    
    return result;
}

std::vector<ChunkingResult> AdvancedChunker::chunk_documents_batch(
    const std::vector<std::tuple<std::string, std::string, std::string, std::unordered_map<std::string, std::string>>>& documents
) {
    std::vector<ChunkingResult> results;
    results.reserve(documents.size());
    
    for (const auto& doc_tuple : documents) {
        const auto& [doc_id, content, title, metadata] = doc_tuple;
        auto result = chunk_document(doc_id, content, title, metadata);
        results.push_back(result);
    }
    
    return results;
}

std::vector<DocumentChunk> AdvancedChunker::filter_chunks_by_quality(
    const std::vector<DocumentChunk>& chunks
) {
    return apply_quality_filtering(chunks);
}

void AdvancedChunker::update_config(const Config& config) {
    config_ = config;
    initialize_components();
}

std::vector<DocumentChunk> AdvancedChunker::apply_quality_filtering(
    const std::vector<DocumentChunk>& chunks
) {
    if (!config_.enable_quality_filtering) {
        return chunks;
    }
    
    std::vector<DocumentChunk> filtered_chunks;
    filtered_chunks.reserve(chunks.size());
    
    for (const auto& chunk : chunks) {
        if (should_include_chunk(chunk)) {
            filtered_chunks.push_back(chunk);
        }
    }
    
    return filtered_chunks;
}

bool AdvancedChunker::should_include_chunk(const DocumentChunk& chunk) const {
    // Check minimum quality score
    if (chunk.quality_score < config_.min_chunk_quality) {
        return false;
    }
    
    // Check minimum information density
    if (chunk.information_density < config_.min_chunk_density) {
        return false;
    }
    
    // Check minimum length (only if content is not empty)
    if (!chunk.content.empty() && chunk.content.length() < config_.min_chunk_length) {
        return false;
    }
    
    // Check maximum length
    if (chunk.content.length() > config_.max_chunk_length) {
        return false;
    }
    
    return true;
}

void AdvancedChunker::calculate_chunk_metrics(DocumentChunk& chunk) const {
    // Calculate word count and unique words
    size_t word_count = 0;
    size_t unique_words = 0;
    std::unordered_set<std::string> words;
    
    std::stringstream ss(chunk.content);
    std::string word;
    while (ss >> word) {
        word_count++;
        words.insert(word);
    }
    
    unique_words = words.size();
    
    // Calculate quality score based on content length and complexity
    if (word_count > 0) {
        // Quality score based on word count and diversity
        double length_factor = std::min(1.0, (double)word_count / 100.0);
        double diversity_factor = std::min(1.0, (double)unique_words / (double)word_count);
        
        chunk.quality_score = (length_factor * 0.6) + (diversity_factor * 0.4);
        chunk.information_density = diversity_factor;
        
        // Determine if chunk is high quality
        chunk.is_high_quality = chunk.quality_score > 0.5 && chunk.information_density > 0.3;
    } else {
        chunk.quality_score = 0.0;
        chunk.information_density = 0.0;
        chunk.is_high_quality = false;
    }
}

} // namespace chunking
} // namespace r3m 