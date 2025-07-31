#include "r3m/formats/processor.hpp"
#include "r3m/utils/text_utils.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

// PDF processing with poppler-cpp
#include <poppler-document.h>
#include <poppler-page.h>

// HTML processing with gumbo-parser
#include <gumbo.h>

namespace r3m {
namespace formats {

FormatProcessor::FormatProcessor() {
    // Initialize default configuration
    encoding_detection_ = true;
    default_encoding_ = "utf-8";
    remove_html_tags_ = true;
    normalize_whitespace_ = true;
    
    // Initialize supported extensions
    plain_text_extensions_ = {".txt", ".md", ".mdx", ".conf", ".log", ".json", ".csv", ".tsv", ".xml", ".yml", ".yaml"};
    pdf_extensions_ = {".pdf"};
    html_extensions_ = {".html", ".htm"};
}

bool FormatProcessor::initialize(const std::unordered_map<std::string, std::string>& config) {
    // Load configuration settings
    auto it = config.find("document_processing.text_processing.encoding_detection");
    if (it != config.end()) {
        encoding_detection_ = (it->second == "true");
    }
    
    it = config.find("document_processing.text_processing.default_encoding");
    if (it != config.end()) {
        default_encoding_ = it->second;
    }
    
    it = config.find("document_processing.text_processing.remove_html_tags");
    if (it != config.end()) {
        remove_html_tags_ = (it->second == "true");
    }
    
    it = config.find("document_processing.text_processing.normalize_whitespace");
    if (it != config.end()) {
        normalize_whitespace_ = (it->second == "true");
    }
    
    return true;
}

FileType FormatProcessor::detect_file_type(const std::string& file_path) const {
    std::string extension = get_file_extension(file_path);
    
    if (std::find(plain_text_extensions_.begin(), plain_text_extensions_.end(), extension) != plain_text_extensions_.end()) {
        return FileType::PLAIN_TEXT;
    } else if (std::find(pdf_extensions_.begin(), pdf_extensions_.end(), extension) != pdf_extensions_.end()) {
        return FileType::PDF;
    } else if (std::find(html_extensions_.begin(), html_extensions_.end(), extension) != html_extensions_.end()) {
        return FileType::HTML;
    }
    
    return FileType::UNSUPPORTED;
}

std::string FormatProcessor::get_file_extension(const std::string& file_path) const {
    return utils::TextUtils::get_file_extension(file_path);
}

bool FormatProcessor::is_supported_file_type(const std::string& file_path) const {
    std::string extension = get_file_extension(file_path);
    
    return std::find(plain_text_extensions_.begin(), plain_text_extensions_.end(), extension) != plain_text_extensions_.end() ||
           std::find(pdf_extensions_.begin(), pdf_extensions_.end(), extension) != pdf_extensions_.end() ||
           std::find(html_extensions_.begin(), html_extensions_.end(), extension) != html_extensions_.end();
}

std::vector<std::string> FormatProcessor::get_supported_extensions() const {
    std::vector<std::string> all_extensions;
    all_extensions.insert(all_extensions.end(), plain_text_extensions_.begin(), plain_text_extensions_.end());
    all_extensions.insert(all_extensions.end(), pdf_extensions_.begin(), pdf_extensions_.end());
    all_extensions.insert(all_extensions.end(), html_extensions_.begin(), html_extensions_.end());
    return all_extensions;
}

std::string FormatProcessor::process_plain_text(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + file_path);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string FormatProcessor::process_pdf(const std::string& file_path) {
    try {
        // Load PDF document
        std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(file_path));
        if (!doc) {
            throw std::runtime_error("Failed to load PDF document");
        }
        
        std::string text_content;
        int num_pages = doc->pages();
        
        // Extract text from each page
        for (int i = 0; i < num_pages; ++i) {
            std::unique_ptr<poppler::page> page(doc->create_page(i));
            if (page) {
                std::string page_text = page->text().to_latin1();
                if (!page_text.empty()) {
                    text_content += page_text + "\n\n";
                }
            }
        }
        
        return text_content;
        
    } catch (const std::exception& e) {
        throw std::runtime_error("PDF processing failed: " + std::string(e.what()));
    }
}

// Helper function to extract text from HTML nodes
void extract_text_from_node(GumboNode* node, std::string& text) {
    if (node->type == GUMBO_NODE_TEXT) {
        text += node->v.text.text;
    } else if (node->type == GUMBO_NODE_ELEMENT) {
        // Skip script and style tags
        if (node->v.element.tag != GUMBO_TAG_SCRIPT && 
            node->v.element.tag != GUMBO_TAG_STYLE) {
            GumboVector* children = &node->v.element.children;
            for (unsigned int i = 0; i < children->length; ++i) {
                extract_text_from_node(static_cast<GumboNode*>(children->data[i]), text);
            }
        }
    }
}

std::string FormatProcessor::process_html(const std::string& file_path) {
    try {
        // Read HTML file
        std::ifstream file(file_path);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open HTML file: " + file_path);
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string html_content = buffer.str();
        
        // Parse HTML with gumbo
        GumboOutput* output = gumbo_parse(html_content.c_str());
        if (!output) {
            throw std::runtime_error("Failed to parse HTML");
        }
        
        // Extract text from the parsed HTML
        std::string text_content;
        extract_text_from_node(output->root, text_content);
        
        // Clean up gumbo output
        gumbo_destroy_output(&kGumboDefaultOptions, output);
        
        return text_content;
        
    } catch (const std::exception& e) {
        throw std::runtime_error("HTML processing failed: " + std::string(e.what()));
    }
}

std::string FormatProcessor::normalize_whitespace(const std::string& text) {
    return utils::TextUtils::normalize_whitespace(text);
}

std::string FormatProcessor::remove_html_tags(const std::string& text) {
    return utils::TextUtils::remove_html_tags(text);
}

std::string FormatProcessor::clean_text_content(const std::string& text) {
    return utils::TextUtils::clean_text_content(text);
}

std::string FormatProcessor::detect_encoding(const std::string& file_path) {
    return utils::TextUtils::detect_encoding(file_path);
}

bool FormatProcessor::is_text_file(const std::string& file_path) const {
    (void)file_path; // Suppress unused parameter warning
    return true; // Simplified for now
}

void FormatProcessor::extract_text_from_node(void* node, std::string& text) {
    // This is a placeholder - the actual implementation is in the process_html method
    (void)node; // Suppress unused parameter warning
    (void)text; // Suppress unused parameter warning
}

} // namespace formats
} // namespace r3m 