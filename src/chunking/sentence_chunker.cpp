#include "r3m/chunking/sentence_chunker.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace r3m {
namespace chunking {

SentenceChunker::SentenceChunker(
    std::shared_ptr<Tokenizer> tokenizer,
    size_t chunk_size,
    size_t chunk_overlap,
    const std::string& return_type
) : tokenizer_(tokenizer), chunk_size_(chunk_size), 
    chunk_overlap_(chunk_overlap), return_type_(return_type) {
}

std::vector<std::string> SentenceChunker::chunk(const std::string& text) const {
    if (text.empty()) {
        return {};
    }
    
    // Split text into sentences
    auto sentences = split_into_sentences(text);
    
    // Merge sentences into chunks
    return merge_sentences_into_chunks(sentences);
}

std::vector<std::string> SentenceChunker::split_into_sentences(const std::string& text) const {
    std::vector<std::string> sentences;
    std::string current_sentence;
    
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        current_sentence += c;
        
        // Check for sentence endings
        if (c == '.' || c == '!' || c == '?') {
            // Look ahead to see if this is really a sentence end
            bool is_sentence_end = true;
            
            // Check for abbreviations (e.g., "Mr.", "Dr.", "etc.")
            if (i > 0 && std::isupper(text[i-1])) {
                // Check if next character is space and previous is uppercase
                if (i + 1 < text.length() && std::isspace(text[i + 1])) {
                    // This might be an abbreviation, check for common patterns
                    std::string prev_word;
                    size_t j = i - 1;
                    while (j > 0 && !std::isspace(text[j]) && std::isalpha(text[j])) {
                        prev_word = text[j] + prev_word;
                        j--;
                    }
                    
                    // Common abbreviations that end with period
                    static const std::vector<std::string> abbreviations = {
                        "Mr", "Mrs", "Ms", "Dr", "Prof", "Sr", "Jr", "St", "Ave", "Blvd",
                        "Rd", "Ln", "Ct", "Pl", "etc", "vs", "i.e", "e.g", "a.m", "p.m"
                    };
                    
                    for (const auto& abbr : abbreviations) {
                        if (prev_word == abbr) {
                            is_sentence_end = false;
                            break;
                        }
                    }
                }
            }
            
            if (is_sentence_end) {
                std::string cleaned = clean_sentence(current_sentence);
                if (!cleaned.empty()) {
                    sentences.push_back(cleaned);
                }
                current_sentence.clear();
            }
        }
    }
    
    // Add any remaining text as a sentence
    if (!current_sentence.empty()) {
        std::string cleaned = clean_sentence(current_sentence);
        if (!cleaned.empty()) {
            sentences.push_back(cleaned);
        }
    }
    
    return sentences;
}

std::vector<std::string> SentenceChunker::merge_sentences_into_chunks(
    const std::vector<std::string>& sentences
) const {
    std::vector<std::string> chunks;
    std::string current_chunk;
    
    for (const auto& sentence : sentences) {
        // Check if adding this sentence would exceed chunk size
        std::string test_chunk = current_chunk;
        if (!test_chunk.empty()) {
            test_chunk += " ";
        }
        test_chunk += sentence;
        
        if (should_start_new_chunk(current_chunk, sentence)) {
            // Start a new chunk
            if (!current_chunk.empty()) {
                chunks.push_back(current_chunk);
            }
            current_chunk = sentence;
        } else {
            // Add to current chunk
            if (!current_chunk.empty()) {
                current_chunk += " ";
            }
            current_chunk += sentence;
        }
    }
    
    // Add the last chunk
    if (!current_chunk.empty()) {
        chunks.push_back(current_chunk);
    }
    
    return chunks;
}

bool SentenceChunker::should_start_new_chunk(
    const std::string& current_chunk,
    const std::string& next_sentence
) const {
    // Calculate tokens for current chunk + next sentence
    std::string combined = current_chunk;
    if (!combined.empty()) {
        combined += " ";
    }
    combined += next_sentence;
    
    size_t combined_tokens = tokenizer_->count_tokens(combined);
    
    // Start new chunk if adding the sentence would exceed the limit
    return combined_tokens > chunk_size_;
}

std::string SentenceChunker::clean_sentence(const std::string& sentence) const {
    std::string cleaned = sentence;
    
    // Trim whitespace safely
    size_t start = cleaned.find_first_not_of(" \t\n\r");
    if (start != std::string::npos) {
        cleaned = cleaned.substr(start);
        size_t end = cleaned.find_last_not_of(" \t\n\r");
        if (end != std::string::npos) {
            cleaned = cleaned.substr(0, end + 1);
        }
    } else {
        cleaned.clear();
    }
    
    // Remove excessive whitespace
    std::string result;
    bool last_was_space = false;
    for (char c : cleaned) {
        if (std::isspace(c)) {
            if (!last_was_space) {
                result += ' ';
                last_was_space = true;
            }
        } else {
            result += c;
            last_was_space = false;
        }
    }
    
    return result;
}

} // namespace chunking
} // namespace r3m 