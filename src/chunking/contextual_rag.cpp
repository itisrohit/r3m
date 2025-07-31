#include "r3m/chunking/contextual_rag.hpp"
#include <sstream>
#include <algorithm>

namespace r3m {
namespace chunking {

ContextualRAG::ContextualRAG(
    std::shared_ptr<Tokenizer> tokenizer,
    size_t reserved_tokens,
    bool use_document_summary,
    bool use_chunk_summary
) : tokenizer_(tokenizer),
    reserved_tokens_(reserved_tokens),
    use_document_summary_(use_document_summary),
    use_chunk_summary_(use_chunk_summary) {
}

std::vector<DocumentChunk> ContextualRAG::add_contextual_summaries(
    std::vector<DocumentChunk>& chunks
) {
    if (chunks.empty()) {
        return chunks;
    }
    
    // Check if document fits in single chunk
    if (document_fits_in_single_chunk(chunks)) {
        // No need for contextual RAG if document fits in single chunk
        for (auto& chunk : chunks) {
            chunk.contextual_rag_reserved_tokens = 0;
            chunk.doc_summary = "";
            chunk.chunk_context = "";
        }
        return chunks;
    }
    
    // Generate document summary if enabled
    std::string document_summary;
    if (use_document_summary_) {
        document_summary = generate_document_summary(chunks);
    }
    
    // Add contextual information to each chunk
    for (auto& chunk : chunks) {
        chunk.contextual_rag_reserved_tokens = reserved_tokens_;
        chunk.doc_summary = document_summary;
        
        if (use_chunk_summary_) {
            chunk.chunk_context = generate_chunk_context(chunk, document_summary);
        } else {
            chunk.chunk_context = "";
        }
    }
    
    return chunks;
}

std::string ContextualRAG::generate_document_summary(const std::vector<DocumentChunk>& chunks) {
    if (chunks.empty()) {
        return "";
    }
    
    // Create document summary prompt
    std::string prompt = create_document_summary_prompt(chunks);
    
    // Simulate LLM response (in real implementation, this would call an actual LLM)
    return simulate_llm_response(prompt);
}

std::string ContextualRAG::generate_chunk_context(
    const DocumentChunk& chunk,
    const std::string& document_summary
) {
    // Create chunk context prompt
    std::string prompt = create_chunk_context_prompt(chunk, document_summary);
    
    // Simulate LLM response
    return simulate_llm_response(prompt);
}

bool ContextualRAG::document_fits_in_single_chunk(const std::vector<DocumentChunk>& chunks) {
    if (chunks.size() <= 1) {
        return true;
    }
    
    // Check if total tokens fit within a reasonable limit
    size_t total_tokens = 0;
    for (const auto& chunk : chunks) {
        total_tokens += count_tokens(chunk.content);
    }
    
    // If document has multiple chunks, it doesn't fit in single chunk
    return false;
}

std::string ContextualRAG::create_document_summary_prompt(const std::vector<DocumentChunk>& chunks) {
    std::stringstream ss;
    
    ss << "Document Summary Task:\n\n";
    ss << "Please provide a concise summary of the following document content:\n\n";
    
    for (size_t i = 0; i < chunks.size(); ++i) {
        ss << "Section " << (i + 1) << ":\n";
        ss << chunks[i].content << "\n\n";
    }
    
    ss << "Summary:";
    
    return ss.str();
}

std::string ContextualRAG::create_chunk_context_prompt(
    const DocumentChunk& chunk,
    const std::string& document_summary
) {
    std::stringstream ss;
    
    ss << "Chunk Context Task:\n\n";
    
    if (!document_summary.empty()) {
        ss << "Document Summary:\n" << document_summary << "\n\n";
    }
    
    ss << "Chunk Content:\n" << chunk.content << "\n\n";
    ss << "Provide context for this chunk within the document:";
    
    return ss.str();
}

std::string ContextualRAG::simulate_llm_response(const std::string& prompt) {
    // This is a simplified simulation of LLM response
    // In a real implementation, this would call an actual LLM API
    
    if (prompt.find("Document Summary Task") != std::string::npos) {
        // Generate document summary
        return "This document contains comprehensive information about the topic with multiple sections covering various aspects.";
    } else if (prompt.find("Chunk Context Task") != std::string::npos) {
        // Generate chunk context
        return "This chunk provides specific details and examples related to the main topic discussed in the document.";
    }
    
    return "Contextual information generated for enhanced retrieval.";
}

size_t ContextualRAG::count_tokens(const std::string& text) const {
    if (!tokenizer_) {
        return text.length() / 4; // Rough approximation
    }
    
    return tokenizer_->count_tokens(text);
}

} // namespace chunking
} // namespace r3m 