#include "r3m/chunking/metadata_processor.hpp"
#include "r3m/chunking/chunk_models.hpp"
#include <algorithm>
#include <sstream>
#include <cctype>

namespace r3m {
namespace chunking {

// Metadata keys to ignore
const std::vector<std::string> MetadataProcessor::IGNORED_METADATA_KEYS = {
    "id", "created_at", "updated_at", "deleted_at", "version", "hash", "checksum",
    "file_size", "file_path", "file_name", "file_extension", "mime_type",
    "encoding", "language", "confidence", "score", "quality", "processed"
};

// Metadata keys that should be treated as semantic (natural language)
const std::vector<std::string> MetadataProcessor::SEMANTIC_METADATA_KEYS = {
    "title", "description", "summary", "abstract", "content", "text", "body",
    "author", "creator", "owner", "subject", "topic", "category", "tags",
    "keywords", "labels", "notes", "comments", "remarks"
};

// Metadata keys that should be treated as keywords (structured)
const std::vector<std::string> MetadataProcessor::KEYWORD_METADATA_KEYS = {
    "type", "format", "source", "origin", "location", "url", "link",
    "status", "state", "priority", "level", "version", "revision",
    "department", "team", "project", "organization", "company"
};

MetadataProcessor::MetadataResult MetadataProcessor::process_metadata(
    const std::unordered_map<std::string, std::string>& metadata,
    bool include_separator) {
    
    MetadataResult result;
    
    if (metadata.empty()) {
        return result;
    }
    
    // Extract semantic and keyword metadata
    std::string semantic_metadata = extract_semantic_metadata(metadata);
    std::string keyword_metadata = extract_keyword_metadata(metadata);
    
    // Add separators if requested
    if (include_separator) {
        if (!semantic_metadata.empty()) {
            semantic_metadata = "\n\n" + semantic_metadata;
        }
        if (!keyword_metadata.empty()) {
            keyword_metadata = "\n\n" + keyword_metadata;
        }
    }
    
    result.semantic_suffix = semantic_metadata;
    result.keyword_suffix = keyword_metadata;
    result.is_valid = !semantic_metadata.empty() || !keyword_metadata.empty();
    
    return result;
}

std::pair<std::string, std::string> MetadataProcessor::get_metadata_suffix_for_document_index(
    const std::unordered_map<std::string, std::string>& metadata,
    bool include_separator) {
    
    auto result = process_metadata(metadata, include_separator);
    return {result.semantic_suffix, result.keyword_suffix};
}

bool MetadataProcessor::is_metadata_too_large(int metadata_tokens, int chunk_token_limit) {
    return metadata_tokens >= chunk_token_limit * MAX_METADATA_PERCENTAGE;
}

std::string MetadataProcessor::clean_metadata_value(const std::string& value) {
    std::string cleaned = value;
    
    // Remove leading/trailing whitespace
    cleaned.erase(0, cleaned.find_first_not_of(" \t\n\r"));
    cleaned.erase(cleaned.find_last_not_of(" \t\n\r") + 1);
    
    // Replace multiple whitespace with single space
    std::string::iterator new_end = std::unique(cleaned.begin(), cleaned.end(),
        [](char a, char b) { return std::isspace(a) && std::isspace(b); });
    cleaned.erase(new_end, cleaned.end());
    
    // Replace tabs and newlines with spaces
    std::replace(cleaned.begin(), cleaned.end(), '\t', ' ');
    std::replace(cleaned.begin(), cleaned.end(), '\n', ' ');
    std::replace(cleaned.begin(), cleaned.end(), '\r', ' ');
    
    return cleaned;
}

std::string MetadataProcessor::extract_semantic_metadata(
    const std::unordered_map<std::string, std::string>& metadata) {
    
    std::vector<std::string> semantic_parts;
    
    for (const auto& [key, value] : metadata) {
        if (should_ignore_metadata_key(key)) {
            continue;
        }
        
        if (is_semantic_metadata_key(key)) {
            std::string cleaned_value = clean_metadata_value(value);
            if (!cleaned_value.empty()) {
                // Format as natural language
                std::string formatted = key + ": " + cleaned_value;
                semantic_parts.push_back(formatted);
            }
        }
    }
    
    if (semantic_parts.empty()) {
        return "";
    }
    
    // Join with newlines for natural language format
    std::ostringstream oss;
    for (size_t i = 0; i < semantic_parts.size(); ++i) {
        if (i > 0) oss << "\n";
        oss << semantic_parts[i];
    }
    
    return oss.str();
}

std::string MetadataProcessor::extract_keyword_metadata(
    const std::unordered_map<std::string, std::string>& metadata) {
    
    std::vector<std::string> keyword_parts;
    
    for (const auto& [key, value] : metadata) {
        if (should_ignore_metadata_key(key)) {
            continue;
        }
        
        if (is_keyword_metadata_key(key)) {
            std::string cleaned_value = clean_metadata_value(value);
            if (!cleaned_value.empty()) {
                // Format as structured data
                std::string formatted = key + "=" + cleaned_value;
                keyword_parts.push_back(formatted);
            }
        }
    }
    
    if (keyword_parts.empty()) {
        return "";
    }
    
    // Join with spaces for keyword format
    std::ostringstream oss;
    for (size_t i = 0; i < keyword_parts.size(); ++i) {
        if (i > 0) oss << " ";
        oss << keyword_parts[i];
    }
    
    return oss.str();
}

bool MetadataProcessor::should_ignore_metadata_key(const std::string& key) {
    std::string lower_key = key;
    std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
    
    return std::find(IGNORED_METADATA_KEYS.begin(), IGNORED_METADATA_KEYS.end(), lower_key) 
           != IGNORED_METADATA_KEYS.end();
}

bool MetadataProcessor::is_semantic_metadata_key(const std::string& key) {
    std::string lower_key = key;
    std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
    
    return std::find(SEMANTIC_METADATA_KEYS.begin(), SEMANTIC_METADATA_KEYS.end(), lower_key) 
           != SEMANTIC_METADATA_KEYS.end();
}

bool MetadataProcessor::is_keyword_metadata_key(const std::string& key) {
    std::string lower_key = key;
    std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
    
    return std::find(KEYWORD_METADATA_KEYS.begin(), KEYWORD_METADATA_KEYS.end(), lower_key) 
           != KEYWORD_METADATA_KEYS.end();
}

} // namespace chunking
} // namespace r3m 