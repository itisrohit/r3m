#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace r3m {
namespace formats {

enum class FileType {
    PLAIN_TEXT,
    PDF,
    HTML,
    UNSUPPORTED
};

class FormatProcessor {
public:
    FormatProcessor();
    ~FormatProcessor() = default;

    bool initialize(const std::unordered_map<std::string, std::string>& config);
    
    // Format detection and processing
    FileType detect_file_type(const std::string& file_path) const;
    std::string get_file_extension(const std::string& file_path) const;
    bool is_supported_file_type(const std::string& file_path) const;
    std::vector<std::string> get_supported_extensions() const;
    
    // Text extraction for different formats
    std::string process_plain_text(const std::string& file_path);
    std::string process_pdf(const std::string& file_path);
    std::string process_html(const std::string& file_path);
    
    // Text cleaning and normalization
    std::string normalize_whitespace(const std::string& text);
    std::string remove_html_tags(const std::string& text);
    std::string clean_text_content(const std::string& text);
    std::string detect_encoding(const std::string& file_path);
    bool is_text_file(const std::string& file_path) const;

private:
    std::vector<std::string> plain_text_extensions_;
    std::vector<std::string> pdf_extensions_;
    std::vector<std::string> html_extensions_;
    
    // Configuration settings
    bool encoding_detection_;
    std::string default_encoding_;
    bool remove_html_tags_;
    bool normalize_whitespace_;
    
    // Helper function for HTML processing
    void extract_text_from_node(void* node, std::string& text);
};

} // namespace formats
} // namespace r3m 