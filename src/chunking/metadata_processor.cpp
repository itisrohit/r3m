#include "r3m/chunking/metadata_processor.hpp"
#include "r3m/chunking/chunk_models.hpp"
#include <algorithm>
#include <sstream>
#include <cctype>

namespace r3m {
namespace chunking {

// Only ignore the specific key that is commonly ignored
const std::vector<std::string> MetadataProcessor::IGNORED_METADATA_KEYS = {
    "ignore_for_qa"
};

// Remove the restrictive semantic/keyword categorization - process all metadata 
const std::vector<std::string> MetadataProcessor::SEMANTIC_METADATA_KEYS = {};
const std::vector<std::string> MetadataProcessor::KEYWORD_METADATA_KEYS = {};

MetadataProcessor::MetadataResult MetadataProcessor::process_metadata(
    const std::unordered_map<std::string, std::string>& metadata,
    bool include_separator) {
    
    MetadataResult result;
    
    if (metadata.empty()) {
        return result;
    }
    
    // Extract all metadata 
    std::string semantic_metadata = extract_all_metadata(metadata);
    std::string keyword_metadata = extract_all_values(metadata);
    
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

// New method to extract all metadata 
std::string MetadataProcessor::extract_all_metadata(
    const std::unordered_map<std::string, std::string>& metadata) {
    
    if (metadata.empty()) {
        return "";
    }
    
    std::ostringstream oss;
    oss << "Metadata:\n";
    
    for (const auto& [key, value] : metadata) {
        if (should_ignore_metadata_key(key)) {
            continue;
        }
        
        std::string cleaned_value = clean_metadata_value(value);
        if (!cleaned_value.empty()) {
            oss << "\t" << key << " - " << cleaned_value << "\n";
        }
    }
    
    std::string result = oss.str();
    // Remove trailing newline
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    
    return result;
}

// New method to extract all values for keyword search
std::string MetadataProcessor::extract_all_values(
    const std::unordered_map<std::string, std::string>& metadata) {
    
    std::vector<std::string> values;
    
    for (const auto& [key, value] : metadata) {
        if (should_ignore_metadata_key(key)) {
            continue;
        }
        
        std::string cleaned_value = clean_metadata_value(value);
        if (!cleaned_value.empty()) {
            values.push_back(cleaned_value);
        }
    }
    
    if (values.empty()) {
        return "";
    }
    
    // Join with spaces for keyword format 
    std::ostringstream oss;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) oss << " ";
        oss << values[i];
    }
    
    return oss.str();
}

std::string MetadataProcessor::extract_semantic_metadata(
    const std::unordered_map<std::string, std::string>& metadata) {
    
    // Use the new method that processes all metadata
    return extract_all_metadata(metadata);
}

std::string MetadataProcessor::extract_keyword_metadata(
    const std::unordered_map<std::string, std::string>& metadata) {
    
    // Use the new method that processes all metadata
    return extract_all_values(metadata);
}

bool MetadataProcessor::should_ignore_metadata_key(const std::string& key) {
    std::string lower_key = key;
    std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
    
    return std::find(IGNORED_METADATA_KEYS.begin(), IGNORED_METADATA_KEYS.end(), lower_key) 
           != IGNORED_METADATA_KEYS.end();
}

bool MetadataProcessor::is_semantic_metadata_key(const std::string& key) {
    // Process all keys now 
    return !should_ignore_metadata_key(key);
}

bool MetadataProcessor::is_keyword_metadata_key(const std::string& key) {
    // Process all keys now 
    return !should_ignore_metadata_key(key);
}

} // namespace chunking
} // namespace r3m 