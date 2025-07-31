#include "r3m/chunking/advanced_chunker.hpp"
#include "r3m/utils/text_processing.hpp"
#include <algorithm>
#include <sstream>
#include <chrono>
#include <unordered_set>

namespace r3m {
namespace chunking {

AdvancedChunker::AdvancedChunker(std::shared_ptr<Tokenizer> tokenizer, const Config& config)
    : tokenizer_(tokenizer), config_(config) {
    initialize_components();
}

void AdvancedChunker::initialize_components() {
    // Initialize sentence chunkers
    blurb_splitter_ = std::make_unique<SentenceChunker>(
        tokenizer_, config_.blurb_size, 0, "texts"
    );
    
    chunk_splitter_ = std::make_unique<SentenceChunker>(
        tokenizer_, config_.chunk_token_limit, config_.chunk_overlap, "texts"
    );
    
    if (config_.enable_multipass) {
        mini_chunk_splitter_ = std::make_unique<SentenceChunker>(
            tokenizer_, config_.mini_chunk_size, 0, "texts"
        );
    }
    
    // Initialize advanced components
    if (config_.enable_multipass) {
        multipass_chunker_ = std::make_unique<MultipassChunker>(
            tokenizer_, true, true, config_.mini_chunk_size, config_.large_chunk_ratio, config_.chunk_token_limit
        );
    }
    
    if (config_.enable_contextual_rag) {
        contextual_rag_ = std::make_unique<ContextualRAG>(
            tokenizer_, config_.contextual_rag_reserved_tokens, true, true
        );
    }
}

ChunkingResult AdvancedChunker::process_document(const DocumentInfo& document) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Step 1: Source-specific handling
    if (document.source_type == "gmail") {
        // Gmail-specific processing
        // In a real implementation, this would have specific logic for Gmail documents
    }
    
    ChunkingResult result;
    
    try {
        // Step 2: Manage token allocation
        auto token_result = manage_tokens(document);
        
        // Step 2: Process sections
        auto chunks = process_sections(document, token_result);
        
        // Step 3: Apply quality filtering
        chunks = apply_quality_filtering(chunks);
        
        // Step 4: Apply multipass indexing if enabled
        if (config_.enable_multipass && multipass_chunker_) {
            auto large_chunks = multipass_chunker_->generate_large_chunks(chunks);
            chunks.insert(chunks.end(), large_chunks.begin(), large_chunks.end());
        }
        
        // Step 5: Apply contextual RAG if enabled
        if (config_.enable_contextual_rag && contextual_rag_) {
            contextual_rag_->add_contextual_summaries(chunks);
        }
        
        // Step 6: Calculate final statistics
        result.chunks = chunks;
        result.total_chunks = chunks.size();
        result.successful_chunks = chunks.size();
        result.failed_chunks = 0;
        
        // Calculate quality metrics
        double total_quality = 0.0;
        double total_density = 0.0;
        size_t high_quality_count = 0;
        
        for (const auto& chunk : chunks) {
            total_quality += chunk.quality_score;
            total_density += chunk.information_density;
            if (chunk.is_high_quality) high_quality_count++;
            
            result.total_title_tokens += chunk.title_tokens;
            result.total_metadata_tokens += chunk.metadata_tokens;
            result.total_content_tokens += static_cast<size_t>(tokenizer_->count_tokens(chunk.content));
            result.total_rag_tokens += chunk.contextual_rag_reserved_tokens;
        }
        
        if (!chunks.empty()) {
            result.avg_quality_score = total_quality / chunks.size();
            result.avg_information_density = total_density / chunks.size();
        }
        result.high_quality_chunks = high_quality_count;
        
    } catch (const std::exception& e) {
        result.failed_chunks = 1;
        result.successful_chunks = 0;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    result.processing_time_ms = duration.count() / 1000.0;
    
    return result;
}

std::vector<ChunkingResult> AdvancedChunker::process_documents(const std::vector<DocumentInfo>& documents) {
    std::vector<ChunkingResult> results;
    results.reserve(documents.size());
    
    for (const auto& document : documents) {
        results.push_back(process_document(document));
    }
    
    return results;
}

AdvancedChunker::TokenManagementResult AdvancedChunker::manage_tokens(const DocumentInfo& document) {
    TokenManagementResult result;
    
    // Step 1: Extract title blurb
    std::string title_blurb = extract_title_blurb(document.title);
    result.title_prefix = title_blurb.empty() ? "" : title_blurb + "\n";
    result.title_tokens = static_cast<int>(tokenizer_->count_tokens(result.title_prefix));
    
    // Step 2: Process metadata
    if (config_.include_metadata && !document.metadata.empty()) {
        auto metadata_result = MetadataProcessor::get_metadata_suffix_for_document_index(
            document.metadata, true
        );
        
        result.metadata_suffix_semantic = metadata_result.first;
        result.metadata_suffix_keyword = metadata_result.second;
        result.metadata_tokens = static_cast<int>(tokenizer_->count_tokens(result.metadata_suffix_semantic));
        
        // Check if metadata is too large
        if (MetadataProcessor::is_metadata_too_large(result.metadata_tokens, config_.chunk_token_limit)) {
            result.metadata_suffix_semantic = "";
            result.metadata_tokens = 0;
            result.metadata_too_large = true;
        }
    }
    
    // Step 3: Check if document fits in single chunk
    if (config_.enable_contextual_rag) {
        int doc_tokens = static_cast<int>(tokenizer_->count_tokens(document.full_content));
        result.single_chunk_fits = (doc_tokens + result.title_tokens + result.metadata_tokens <= config_.chunk_token_limit);
        
        // Expand context size based on whether chunk context and doc summary are used
        if (!result.single_chunk_fits && !AVERAGE_SUMMARY_EMBEDDINGS) {
            result.contextual_rag_reserved_tokens = config_.contextual_rag_reserved_tokens;
        }
    }
    
    // Step 4: Calculate content token limit
    result.content_token_limit = config_.chunk_token_limit - result.title_tokens - result.metadata_tokens - result.contextual_rag_reserved_tokens;
    
    // Step 5: Check minimum content requirement
    if (result.content_token_limit <= config_.chunk_min_content) {
        // Don't do contextual RAG if not enough content
        result.contextual_rag_reserved_tokens = 0;
        result.content_token_limit = config_.chunk_token_limit - result.title_tokens - result.metadata_tokens;
        
        // If still not enough content, revert to full chunk
        if (result.content_token_limit <= config_.chunk_min_content) {
            result.content_token_limit = config_.chunk_token_limit;
            result.title_prefix = "";
            result.metadata_suffix_semantic = "";
            result.title_tokens = 0;
            result.metadata_tokens = 0;
        }
    }
    
    return result;
}

std::string AdvancedChunker::extract_title_blurb(const std::string& title) {
    if (title.empty()) return "";
    
    // Use blurb splitter to get first sentence(s)
    auto blurbs = blurb_splitter_->chunk(title);
    return blurbs.empty() ? title : blurbs[0];
}

std::vector<DocumentChunk> AdvancedChunker::process_sections(
    const DocumentInfo& document,
    const TokenManagementResult& token_result) {
    
    // Use the new advanced section combination logic
    return process_sections_with_combinations(document, token_result);
}

std::vector<DocumentChunk> AdvancedChunker::process_sections_with_combinations(
    const DocumentInfo& document,
    const TokenManagementResult& token_result) {
    
    std::vector<DocumentChunk> chunks;
    std::unordered_map<int, std::string> link_offsets;
    std::string chunk_text = "";
    int chunk_id = 0;
    
    // Use text processing utilities directly
    
    for (size_t section_idx = 0; section_idx < document.sections.size(); ++section_idx) {
        const auto& section = document.sections[section_idx];
        
        // Get section text and clean it
        std::string section_text = utils::TextProcessing::clean_text(section.content);
        std::string section_link_text = section.link;
        std::string image_url = section.image_file_id;
        
        // Skip empty sections
        if (section_text.empty() && (document.title.empty() || section_idx > 0)) {
            continue;
        }
        
        // CASE 1: If this section has an image, force a separate chunk
        if (!image_url.empty()) {
            // First, finalize any existing text chunk
            if (!chunk_text.empty()) {
                auto chunk = create_chunk_from_section(
                    DocumentSection(chunk_text, ""), chunk_id++, document.document_id,
                    token_result.title_prefix, token_result.metadata_suffix_semantic,
                    token_result.metadata_suffix_keyword, token_result.content_token_limit,
                    document.source_type, document.semantic_identifier, false
                );
                chunks.push_back(chunk);
                chunk_text = "";
                link_offsets.clear();
            }
            
            // Create a chunk specifically for this image section
            DocumentSection image_section(section_text, section_link_text);
            image_section.image_file_id = image_url;
            auto chunk = create_chunk_from_section(
                image_section, chunk_id++, document.document_id,
                token_result.title_prefix, token_result.metadata_suffix_semantic,
                token_result.metadata_suffix_keyword, token_result.content_token_limit,
                document.source_type, document.semantic_identifier, false
            );
            chunks.push_back(chunk);
            continue;
        }
        
        // CASE 2: Normal text section
        int section_token_count = static_cast<int>(tokenizer_->count_tokens(section_text));
        
        // If the section is large on its own, split it separately
        if (section_token_count > token_result.content_token_limit) {
            // Finalize existing chunk
            if (!chunk_text.empty()) {
                auto chunk = create_chunk_from_section(
                    DocumentSection(chunk_text, ""), chunk_id++, document.document_id,
                    token_result.title_prefix, token_result.metadata_suffix_semantic,
                    token_result.metadata_suffix_keyword, token_result.content_token_limit,
                    document.source_type, document.semantic_identifier, false
                );
                chunks.push_back(chunk);
                chunk_text = "";
                link_offsets.clear();
            }
            
            // Split the oversized section
            auto split_texts = chunk_splitter_->chunk(section_text);
            for (size_t i = 0; i < split_texts.size(); ++i) {
                const auto& split_text = split_texts[i];
                
                // Check if even the split text is too big (STRICT_CHUNK_TOKEN_LIMIT)
                if (STRICT_CHUNK_TOKEN_LIMIT && 
                    static_cast<int>(tokenizer_->count_tokens(split_text)) > token_result.content_token_limit) {
                    auto smaller_chunks = split_oversized_chunk(split_text, token_result.content_token_limit);
                    for (size_t j = 0; j < smaller_chunks.size(); ++j) {
                        DocumentSection sub_section(smaller_chunks[j], section_link_text);
                        auto chunk = create_chunk_from_section(
                            sub_section, chunk_id++, document.document_id,
                            token_result.title_prefix, token_result.metadata_suffix_semantic,
                            token_result.metadata_suffix_keyword, token_result.content_token_limit,
                            document.source_type, document.semantic_identifier, (j != 0)
                        );
                        chunks.push_back(chunk);
                    }
                } else {
                    DocumentSection sub_section(split_text, section_link_text);
                    auto chunk = create_chunk_from_section(
                        sub_section, chunk_id++, document.document_id,
                        token_result.title_prefix, token_result.metadata_suffix_semantic,
                        token_result.metadata_suffix_keyword, token_result.content_token_limit,
                        document.source_type, document.semantic_identifier, (i != 0)
                    );
                    chunks.push_back(chunk);
                }
            }
            continue;
        }
        
        // CASE 3: Try to combine sections
        int current_token_count = static_cast<int>(tokenizer_->count_tokens(chunk_text));
        int current_offset = static_cast<int>(utils::TextProcessing::shared_precompare_cleanup(chunk_text).length());
        int next_section_tokens = static_cast<int>(tokenizer_->count_tokens(utils::TextProcessing::SECTION_SEPARATOR)) + section_token_count;
        
        if (next_section_tokens + current_token_count <= token_result.content_token_limit) {
            // Can combine sections
            if (!chunk_text.empty()) {
                chunk_text += utils::TextProcessing::SECTION_SEPARATOR; // "\n\n"
            }
            chunk_text += section_text;
            link_offsets[current_offset] = section_link_text;
        } else {
            // Finalize existing chunk and start new one
            if (!chunk_text.empty()) {
                auto chunk = create_chunk_from_section(
                    DocumentSection(chunk_text, ""), chunk_id++, document.document_id,
                    token_result.title_prefix, token_result.metadata_suffix_semantic,
                    token_result.metadata_suffix_keyword, token_result.content_token_limit,
                    document.source_type, document.semantic_identifier, false
                );
                // Set link offsets
                chunk.source_links = link_offsets;
                chunks.push_back(chunk);
            }
            
            // Start new chunk
            link_offsets = {{0, section_link_text}};
            chunk_text = section_text;
        }
    }
    
    // Finalize any leftover text chunk
    if (!chunk_text.empty() || chunks.empty()) {
        auto chunk = create_chunk_from_section(
            DocumentSection(chunk_text, ""), chunk_id++, document.document_id,
            token_result.title_prefix, token_result.metadata_suffix_semantic,
            token_result.metadata_suffix_keyword, token_result.content_token_limit,
            document.source_type, document.semantic_identifier, false
        );
        // Set link offsets (safe default if empty)
        if (!link_offsets.empty()) {
            chunk.source_links = link_offsets;
        } else {
            chunk.source_links = {{0, ""}};
        }
        chunks.push_back(chunk);
    }
    
    return chunks;
}

std::vector<std::string> AdvancedChunker::split_oversized_chunk(
    const std::string& text,
    int content_token_limit) {

    std::vector<std::string> chunks;
    std::vector<std::string> tokens;
    
    // Simple tokenization by whitespace
    std::istringstream iss(text);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    int start = 0;
    while (start < static_cast<int>(tokens.size())) {
        int end = std::min(start + content_token_limit, static_cast<int>(tokens.size()));
        
        std::string chunk_text;
        for (int i = start; i < end; ++i) {
            if (!chunk_text.empty()) chunk_text += " ";
            chunk_text += tokens[i];
        }
        
        chunks.push_back(chunk_text);
        start = end;
    }
    
    return chunks;
}

std::vector<AdvancedChunker::DocumentSection> AdvancedChunker::handle_image_sections(
    const std::vector<DocumentSection>& sections,
    int content_token_limit) {

    std::vector<DocumentSection> processed_sections;
    
    for (const auto& section : sections) {
        // If section has an image, force it to be a separate chunk
        if (!section.image_file_id.empty()) {
            // Add the image section as-is
            processed_sections.push_back(section);
        } else {
            // For text sections, check if they need splitting
            int section_tokens = static_cast<int>(tokenizer_->count_tokens(section.content));
            
            if (section_tokens <= content_token_limit) {
                processed_sections.push_back(section);
            } else {
                // Split oversized text sections
                auto sub_chunks = chunk_splitter_->chunk(section.content);
                for (const auto& sub_chunk : sub_chunks) {
                    DocumentSection sub_section;
                    sub_section.content = sub_chunk;
                    sub_section.link = section.link;
                    processed_sections.push_back(sub_section);
                }
            }
        }
    }
    
    return processed_sections;
}

DocumentChunk AdvancedChunker::create_chunk_from_section(
    const DocumentSection& section,
    int chunk_id,
    const std::string& document_id,
    const std::string& title_prefix,
    const std::string& metadata_suffix_semantic,
    const std::string& metadata_suffix_keyword,
    int content_token_limit,
    const std::string& source_type,
    const std::string& semantic_identifier,
    bool is_continuation) {

    DocumentChunk chunk;

    // Basic chunk properties
    chunk.chunk_id = chunk_id;
    chunk.document_id = document_id;
    chunk.title_prefix = title_prefix;
    chunk.metadata_suffix_semantic = metadata_suffix_semantic;
    chunk.metadata_suffix_keyword = metadata_suffix_keyword;
    chunk.content = section.content;
    chunk.source_type = source_type;
    chunk.semantic_identifier = semantic_identifier;
    chunk.section_continuation = is_continuation;

    // Token management
    chunk.title_tokens = static_cast<int>(tokenizer_->count_tokens(title_prefix));
    chunk.metadata_tokens = static_cast<int>(tokenizer_->count_tokens(metadata_suffix_semantic));
    chunk.content_token_limit = content_token_limit;

    // Section properties
    if (!section.link.empty()) {
        chunk.source_links[0] = section.link;
    }
    chunk.image_file_id = section.image_file_id;

    // Extract blurb from content
    auto blurbs = blurb_splitter_->chunk(section.content);
    chunk.blurb = blurbs.empty() ? section.content.substr(0, 100) : blurbs[0];

    // Calculate quality metrics
    calculate_chunk_quality(chunk);

    return chunk;
}

std::vector<DocumentChunk> AdvancedChunker::apply_quality_filtering(const std::vector<DocumentChunk>& chunks) {
    std::vector<DocumentChunk> filtered_chunks;
    
    for (const auto& chunk : chunks) {
        // Basic quality checks
        bool is_valid = true;
        
        // Check minimum content length
        if (chunk.content.length() < 50) {
            is_valid = false;
        }
        
        // Check quality score
        if (chunk.quality_score < 0.3) {
            is_valid = false;
        }
        
        // Check information density
        if (chunk.information_density < 0.1) {
            is_valid = false;
        }
        
        if (is_valid) {
            filtered_chunks.push_back(chunk);
        }
    }
    
    return filtered_chunks;
}

void AdvancedChunker::calculate_chunk_quality(DocumentChunk& chunk) {
    // Calculate quality score (0.0 - 1.0)
    double length_factor = std::min(1.0, static_cast<double>(chunk.content.length()) / 1000.0);
    double word_diversity = calculate_word_diversity(chunk.content);
    double sentence_structure = calculate_sentence_structure(chunk.content);
    double information_density = calculate_information_density(chunk.content);
    
    chunk.quality_score = (length_factor * 0.3 + word_diversity * 0.3 + 
                          sentence_structure * 0.2 + information_density * 0.2);
    
    chunk.information_density = information_density;
    chunk.is_high_quality = chunk.quality_score >= 0.7;
}

double AdvancedChunker::calculate_word_diversity(const std::string& text) {
    // Simple word diversity calculation
    std::unordered_set<std::string> unique_words;
    std::istringstream iss(text);
    std::string word;
    
    while (iss >> word) {
        unique_words.insert(word);
    }
    
    return std::min(1.0, static_cast<double>(unique_words.size()) / 100.0);
}

double AdvancedChunker::calculate_sentence_structure(const std::string& text) {
    // Simple sentence structure calculation
    int sentences = 0;
    int words = 0;
    
    std::istringstream iss(text);
    std::string word;
    
    while (iss >> word) {
        words++;
        if (word.find('.') != std::string::npos || 
            word.find('!') != std::string::npos || 
            word.find('?') != std::string::npos) {
            sentences++;
        }
    }
    
    if (sentences == 0) return 0.0;
    double avg_sentence_length = static_cast<double>(words) / sentences;
    return std::min(1.0, avg_sentence_length / 20.0);
}

double AdvancedChunker::calculate_information_density(const std::string& text) {
    // Simple information density calculation
    std::unordered_set<char> chars;
    
    for (char c : text) {
        if (std::isalnum(c)) {
            chars.insert(c);
        }
    }
    
    return std::min(1.0, static_cast<double>(chars.size()) / 50.0);
}

void AdvancedChunker::update_config(const Config& config) {
    config_ = config;
    initialize_components();
}

} // namespace chunking
} // namespace r3m 