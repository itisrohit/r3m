#include "r3m/chunking/section_processing/section_processor.hpp"
#include "r3m/chunking/quality_assessment/quality_calculator.hpp"
#include "r3m/utils/text_processing.hpp"
#include <algorithm>
#include <cctype>

namespace r3m {
namespace chunking {
namespace section_processing {

SectionProcessor::SectionProcessor(
    std::shared_ptr<Tokenizer> tokenizer)
    : tokenizer_(tokenizer), optimized_cache_(std::make_unique<token_management::OptimizedTokenCache>(tokenizer)) {
    
    // Initialize chunk splitter for oversized sections
    chunk_splitter_ = std::make_unique<SentenceChunker>(
        tokenizer, 2048, 0, "texts"
    );
}

std::vector<DocumentChunk> SectionProcessor::process_sections_with_combinations(
    const std::vector<DocumentSection>& sections,
    const TokenManagementResult& token_result,
    const std::string& document_id,
    const std::string& source_type,
    const std::string& semantic_identifier) {
    
    std::vector<DocumentChunk> chunks;
    chunks.reserve(sections.size() + 1); // Pre-allocate for efficiency
    
    std::unordered_map<int, std::string> link_offsets;
    std::string chunk_text("");
    chunk_text.reserve(token_result.content_token_limit * 4); // Pre-allocate string capacity
    
    int chunk_id = 0;
    
    // Pre-cache common strings using string_view
    const std::string_view section_separator = utils::TextProcessing::SECTION_SEPARATOR;
    const int separator_tokens = optimized_cache_->get_token_count(section_separator);
    
    // Pre-allocate vectors for efficiency
    std::vector<std::string> section_texts;
    std::vector<int> section_token_counts;
    section_texts.reserve(sections.size());
    section_token_counts.reserve(sections.size());
    
    // Pre-process all sections to avoid repeated operations
    for (const auto& section : sections) {
        std::string cleaned_text = utils::TextProcessing::clean_text(section.content);
        if (!cleaned_text.empty()) {
            section_texts.push_back(cleaned_text);
            section_token_counts.push_back(optimized_cache_->get_token_count(cleaned_text));
        } else {
            section_texts.push_back("");
            section_token_counts.push_back(0);
        }
    }
    
    for (size_t section_idx = 0; section_idx < sections.size(); ++section_idx) {
        const auto& section = sections[section_idx];
        const auto& section_text = section_texts[section_idx];
        const int section_token_count = section_token_counts[section_idx];
        
        // Skip empty sections
        if (section_text.empty()) {
            continue;
        }
        
        std::string_view section_link_text = section.link;
        std::string_view image_url = section.image_file_id;
        
        // CASE 1: If this section has an image, force a separate chunk
        if (!image_url.empty()) {
            // First, finalize any existing text chunk
            if (!chunk_text.empty()) {
                auto chunk = create_chunk_from_section(
                    DocumentSection(std::string(chunk_text), ""), chunk_id++, document_id,
                    token_result.title_prefix, token_result.metadata_suffix_semantic,
                    token_result.metadata_suffix_keyword, token_result.content_token_limit,
                    source_type, semantic_identifier, false
                );
                chunks.push_back(chunk);
                chunk_text.clear();
                chunk_text.reserve(token_result.content_token_limit * 4);
                link_offsets.clear();
            }
            
            // Create a chunk specifically for this image section
            DocumentSection image_section{std::string(section_text), std::string(section_link_text)};
            image_section.image_file_id = std::string(image_url);
            auto chunk = create_chunk_from_section(
                image_section, chunk_id++, document_id,
                token_result.title_prefix, token_result.metadata_suffix_semantic,
                token_result.metadata_suffix_keyword, token_result.content_token_limit,
                source_type, semantic_identifier, false
            );
            chunks.push_back(chunk);
            continue;
        }
        
        // CASE 2: Normal text section - use pre-computed token count
        if (section_token_count > token_result.content_token_limit) {
            // Finalize existing chunk
            if (!chunk_text.empty()) {
                auto chunk = create_chunk_from_section(
                    DocumentSection(std::string(chunk_text), ""), chunk_id++, document_id,
                    token_result.title_prefix, token_result.metadata_suffix_semantic,
                    token_result.metadata_suffix_keyword, token_result.content_token_limit,
                    source_type, semantic_identifier, false
                );
                chunk.source_links = link_offsets;
                chunks.push_back(chunk);
                chunk_text.clear();
                chunk_text.reserve(token_result.content_token_limit * 4);
                link_offsets.clear();
            }
            
            // Split the oversized section using optimized approach
            auto split_texts = split_oversized_chunk_optimized(std::string(section_text), token_result.content_token_limit);
            for (size_t i = 0; i < split_texts.size(); ++i) {
                const auto& split_text = split_texts[i];
                
                // Check if even the split text is too big (STRICT_CHUNK_TOKEN_LIMIT)
                if (optimized_cache_->get_token_count(split_text) > token_result.content_token_limit) {
                    auto smaller_chunks = split_oversized_chunk_optimized(split_text, token_result.content_token_limit);
                    for (size_t j = 0; j < smaller_chunks.size(); ++j) {
                        DocumentSection sub_section(smaller_chunks[j], std::string(section_link_text));
                        auto chunk = create_chunk_from_section(
                            sub_section, chunk_id++, document_id,
                            token_result.title_prefix, token_result.metadata_suffix_semantic,
                            token_result.metadata_suffix_keyword, token_result.content_token_limit,
                            source_type, semantic_identifier, (j != 0)
                        );
                        chunks.push_back(chunk);
                    }
                } else {
                    DocumentSection sub_section(split_text, std::string(section_link_text));
                    auto chunk = create_chunk_from_section(
                        sub_section, chunk_id++, document_id,
                        token_result.title_prefix, token_result.metadata_suffix_semantic,
                        token_result.metadata_suffix_keyword, token_result.content_token_limit,
                        source_type, semantic_identifier, (i != 0)
                    );
                    chunks.push_back(chunk);
                }
            }
            continue;
        }
        
        // CASE 3: Try to combine sections - use pre-computed token counts
        int current_token_count = chunk_text.empty() ? 0 : optimized_cache_->get_token_count(chunk_text);
        int current_offset = static_cast<int>(utils::TextProcessing::shared_precompare_cleanup(chunk_text).length());
        int next_section_tokens = separator_tokens + section_token_count;
        
        if (next_section_tokens + current_token_count <= token_result.content_token_limit) {
            // Can combine sections - use efficient string operations
            if (!chunk_text.empty()) {
                chunk_text += section_separator;
            }
            chunk_text += section_text;
            link_offsets[current_offset] = std::string(section_link_text);
        } else {
            // Finalize existing chunk and start new one
            if (!chunk_text.empty()) {
                auto chunk = create_chunk_from_section(
                    DocumentSection(std::string(chunk_text), ""), chunk_id++, document_id,
                    token_result.title_prefix, token_result.metadata_suffix_semantic,
                    token_result.metadata_suffix_keyword, token_result.content_token_limit,
                    source_type, semantic_identifier, false
                );
                chunk.source_links = link_offsets;
                chunks.push_back(chunk);
            }
            
            // Start new chunk - use move semantics for efficiency
            link_offsets = {{0, std::string(section_link_text)}};
            chunk_text = section_text;
        }
    }
    
    // Finalize any leftover text chunk
    if (!chunk_text.empty() || chunks.empty()) {
        auto chunk = create_chunk_from_section(
            DocumentSection(std::string(chunk_text), ""), chunk_id++, document_id,
            token_result.title_prefix, token_result.metadata_suffix_semantic,
            token_result.metadata_suffix_keyword, token_result.content_token_limit,
            source_type, semantic_identifier, false
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

std::vector<DocumentSection> SectionProcessor::split_oversized_sections(
    const std::vector<DocumentSection>& sections,
    int content_token_limit) {
    
    std::vector<DocumentSection> split_sections;
    
    for (const auto& section : sections) {
        int section_tokens = static_cast<int>(tokenizer_->count_tokens(section.content));
        
        if (section_tokens <= content_token_limit) {
            split_sections.push_back(section);
        } else {
            // Split oversized section
            auto sub_chunks = chunk_splitter_->chunk(section.content);
            for (const auto& sub_chunk : sub_chunks) {
                DocumentSection sub_section;
                sub_section.content = sub_chunk;
                sub_section.link = section.link;
                sub_section.image_file_id = section.image_file_id;
                split_sections.push_back(sub_section);
            }
        }
    }
    
    return split_sections;
}

std::vector<std::string> SectionProcessor::split_oversized_chunk(
    const std::string& text,
    int content_token_limit) {
    
    std::vector<std::string> chunks;
    chunks.reserve((text.length() / (content_token_limit * 4)) + 1); // Pre-allocate
    
    // Use string_view for efficiency and avoid copying
    std::string_view text_view(text);
    std::vector<std::string_view> tokens;
    tokens.reserve(content_token_limit * 2);
    
    // Simple tokenization by whitespace using string_view
    size_t start = 0;
    size_t end = 0;
    
    while (start < text_view.length()) {
        // Skip leading whitespace
        while (start < text_view.length() && std::isspace(text_view[start])) {
            start++;
        }
        
        if (start >= text_view.length()) break;
        
        // Find end of token
        end = start;
        while (end < text_view.length() && !std::isspace(text_view[end])) {
            end++;
        }
        
        tokens.push_back(text_view.substr(start, end - start));
        start = end;
    }
    
    // Build chunks efficiently
    size_t token_start = 0;
    while (token_start < tokens.size()) {
        size_t token_end = std::min(token_start + static_cast<size_t>(content_token_limit), tokens.size());
        
        // Calculate approximate string size for pre-allocation
        size_t chunk_size = 0;
        for (size_t i = token_start; i < token_end; ++i) {
            chunk_size += tokens[i].length() + 1; // +1 for space
        }
        
        std::string chunk_text;
        chunk_text.reserve(chunk_size);
        
        for (size_t i = token_start; i < token_end; ++i) {
            if (!chunk_text.empty()) chunk_text += ' ';
            chunk_text += tokens[i];
        }
        
        chunks.push_back(std::move(chunk_text));
        token_start = token_end;
    }
    
    return chunks;
}

std::vector<std::string> SectionProcessor::split_oversized_chunk_optimized(
    const std::string& text,
    int content_token_limit) {
    
    // Use direct tokenization for maximum performance
    auto tokens = tokenizer_->tokenize(text);
    
    std::vector<std::string> chunks;
    chunks.reserve((tokens.size() / content_token_limit) + 1);
    
    size_t start = 0;
    while (start < tokens.size()) {
        size_t end = std::min(start + static_cast<size_t>(content_token_limit), tokens.size());
        
        // Pre-calculate the size needed for this chunk
        size_t chunk_size = 0;
        for (size_t i = start; i < end; ++i) {
            chunk_size += tokens[i].length() + 1; // +1 for space
        }
        
        std::string chunk_text;
        chunk_text.reserve(chunk_size);
        
        for (size_t i = start; i < end; ++i) {
            if (!chunk_text.empty()) chunk_text += ' ';
            chunk_text += tokens[i];
        }
        
        chunks.push_back(std::move(chunk_text));
        start = end;
    }
    
    return chunks;
}

std::vector<DocumentSection> SectionProcessor::handle_image_sections(
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

DocumentChunk SectionProcessor::create_chunk_from_section(
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
    
    // Extract blurb from content (simplified version)
    chunk.blurb = section.content.substr(0, std::min(100UL, section.content.length()));
    
    // Calculate quality metrics
    chunk.quality_score = quality_assessment::QualityCalculator::calculate_quality_score(chunk.content);
    chunk.information_density = quality_assessment::QualityCalculator::calculate_information_density(chunk.content);
    chunk.is_high_quality = chunk.quality_score >= 0.7;
    
    return chunk;
}

} // namespace section_processing
} // namespace chunking
} // namespace r3m 