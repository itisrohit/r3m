#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <chrono>
#include "r3m/core/document_processor.hpp"

using namespace r3m::core;

int main() {
    std::filesystem::create_directories("../data");

    // Set up processor with advanced features
    DocumentProcessor processor;
    std::unordered_map<std::string, std::string> config;
    config["document_processing.max_file_size"] = "100MB";
    config["document_processing.max_text_length"] = "1000000";
    config["document_processing.text_processing.encoding_detection"] = "true";
    config["document_processing.text_processing.default_encoding"] = "utf-8";
    config["document_processing.text_processing.remove_html_tags"] = "true";
    config["document_processing.text_processing.normalize_whitespace"] = "true";
    config["document_processing.text_processing.extract_metadata"] = "true";
    config["document_processing.batch_size"] = "8";
    config["document_processing.max_workers"] = "4";
    
    // Quality filtering settings
    config["document_processing.quality_filtering.enabled"] = "true";
    config["document_processing.quality_filtering.min_content_quality_score"] = "0.3";
    config["document_processing.quality_filtering.min_information_density"] = "0.1";
    config["document_processing.quality_filtering.min_content_length"] = "50";
    config["document_processing.quality_filtering.max_content_length"] = "1000000";
    config["document_processing.quality_filtering.filter_empty_documents"] = "true";
    config["document_processing.quality_filtering.filter_low_quality_documents"] = "true";
    
    processor.initialize(config);

    std::cout << "R3M Advanced Document Processing Test\n";
    std::cout << "=====================================\n\n";

    // Test 1: High-quality document
    std::cout << "1. High-Quality Document Test\n";
    std::cout << "-----------------------------\n";
    
    std::string high_quality_content = R"(
This is a comprehensive technical document about R3M document processing system.

The R3M system implements advanced document processing capabilities including:
- Parallel processing with thread pools for improved performance
- Batch processing with configurable sizes following industry standards
- Quality assessment algorithms for content filtering
- Information density calculation for content evaluation
- Multi-format support including PDF, HTML, and plain text files

The system achieves 2x speedup in parallel processing tests and provides
comprehensive quality metrics for document assessment. Technical specifications
include C++20 implementation with cross-platform compatibility.

Key features include real-time statistics tracking, configuration-driven
architecture, and robust error handling mechanisms.
)";

    std::string high_quality_file = "../data/high_quality_doc.txt";
    std::ofstream hq_file(high_quality_file);
    hq_file << high_quality_content;
    hq_file.close();

    DocumentResult hq_result = processor.process_document(high_quality_file);
    
    std::cout << "High-quality document processing:\n";
    std::cout << "  Success: " << (hq_result.processing_success ? "YES" : "NO") << "\n";
    std::cout << "  Processing time: " << hq_result.processing_time_ms << " ms\n";
    std::cout << "  Content quality score: " << hq_result.content_quality_score << "\n";
    std::cout << "  Information density: " << hq_result.information_density << "\n";
    std::cout << "  Is high quality: " << (hq_result.is_high_quality ? "YES" : "NO") << "\n";
    std::cout << "  Quality reason: " << hq_result.quality_reason << "\n";
    std::cout << "  Text length: " << hq_result.text_content.length() << " characters\n\n";

    // Test 2: Low-quality document
    std::cout << "2. Low-Quality Document Test\n";
    std::cout << "----------------------------\n";
    
    std::string low_quality_content = "Short text. Not much info. Basic content.";
    
    std::string low_quality_file = "../data/low_quality_doc.txt";
    std::ofstream lq_file(low_quality_file);
    lq_file << low_quality_content;
    lq_file.close();

    DocumentResult lq_result = processor.process_document(low_quality_file);
    
    std::cout << "Low-quality document processing:\n";
    std::cout << "  Success: " << (lq_result.processing_success ? "YES" : "NO") << "\n";
    std::cout << "  Processing time: " << lq_result.processing_time_ms << " ms\n";
    std::cout << "  Content quality score: " << lq_result.content_quality_score << "\n";
    std::cout << "  Information density: " << lq_result.information_density << "\n";
    std::cout << "  Is high quality: " << (lq_result.is_high_quality ? "YES" : "NO") << "\n";
    std::cout << "  Quality reason: " << lq_result.quality_reason << "\n";
    std::cout << "  Text length: " << lq_result.text_content.length() << " characters\n\n";

    // Test 3: Empty document
    std::cout << "3. Empty Document Test\n";
    std::cout << "---------------------\n";
    
    std::string empty_file = "../data/empty_doc.txt";
    std::ofstream empty_file_stream(empty_file);
    empty_file_stream << "";
    empty_file_stream.close();

    DocumentResult empty_result = processor.process_document(empty_file);
    
    std::cout << "Empty document processing:\n";
    std::cout << "  Success: " << (empty_result.processing_success ? "YES" : "NO") << "\n";
    std::cout << "  Processing time: " << empty_result.processing_time_ms << " ms\n";
    std::cout << "  Content quality score: " << empty_result.content_quality_score << "\n";
    std::cout << "  Information density: " << empty_result.information_density << "\n";
    std::cout << "  Is high quality: " << (empty_result.is_high_quality ? "YES" : "NO") << "\n";
    std::cout << "  Quality reason: " << empty_result.quality_reason << "\n";
    std::cout << "  Text length: " << empty_result.text_content.length() << " characters\n\n";

    // Test 4: Mixed quality batch processing with filtering
    std::cout << "4. Batch Processing with Quality Filtering\n";
    std::cout << "-----------------------------------------\n";
    
    std::vector<std::string> mixed_files;
    
    // Create various quality documents
    std::vector<std::pair<std::string, std::string>> test_docs = {
        {"good_doc1.txt", "This is a well-structured technical document with comprehensive information about R3M system architecture and implementation details."},
        {"good_doc2.txt", "Another high-quality document containing detailed specifications and technical requirements for the document processing pipeline."},
        {"mediocre_doc.txt", "Some content here. Not very detailed. Basic information."},
        {"poor_doc.txt", "Short."},
        {"empty_doc.txt", ""},
        {"technical_doc.txt", "R3M system implements advanced features: parallel processing (2x speedup), batch processing (16 docs/batch), quality filtering (0.3+ score), information density assessment (0.1+ density), multi-format support (PDF/HTML/text), cross-platform compatibility (C++20), configuration-driven architecture (YAML), real-time statistics tracking, robust error handling, thread-safe operations, memory-efficient processing."}
    };
    
    for (const auto& [filename, content] : test_docs) {
        std::string filepath = "../data/" + filename;
        std::ofstream file(filepath);
        file << content;
        file.close();
        mixed_files.push_back(filepath);
    }
    
    // Process with filtering
    auto start_time = std::chrono::high_resolution_clock::now();
    auto filtered_results = processor.process_documents_with_filtering(mixed_files);
    auto end_time = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Batch processing with filtering results:\n";
    std::cout << "  Total files: " << mixed_files.size() << "\n";
    std::cout << "  Filtered results: " << filtered_results.size() << "\n";
    std::cout << "  Processing time: " << duration.count() << " ms\n";
    std::cout << "  Average time per file: " << (duration.count() / mixed_files.size()) << " ms\n\n";
    
    // Show quality details for each result
    for (size_t i = 0; i < filtered_results.size(); ++i) {
        const auto& result = filtered_results[i];
        std::cout << "  Document " << (i + 1) << " (" << result.file_name << "):\n";
        std::cout << "    Quality score: " << result.content_quality_score << "\n";
        std::cout << "    Information density: " << result.information_density << "\n";
        std::cout << "    High quality: " << (result.is_high_quality ? "YES" : "NO") << "\n";
        std::cout << "    Text length: " << result.text_content.length() << " chars\n\n";
    }

    // Test 5: Statistics
    std::cout << "5. Processing Statistics\n";
    std::cout << "------------------------\n";
    
    auto stats = processor.get_processing_stats();
    std::cout << "Total files processed: " << stats.total_files_processed << "\n";
    std::cout << "Successful processing: " << stats.successful_processing << "\n";
    std::cout << "Failed processing: " << stats.failed_processing << "\n";
    std::cout << "Filtered out: " << stats.filtered_out << "\n";
    std::cout << "Average processing time: " << stats.avg_processing_time_ms << " ms\n";
    std::cout << "Total text extracted: " << stats.total_text_extracted << " characters\n";
    std::cout << "Average content quality score: " << stats.avg_content_quality_score << "\n";
    std::cout << "Text files processed: " << stats.text_files_processed << "\n";
    std::cout << "PDF files processed: " << stats.pdf_files_processed << "\n";
    std::cout << "HTML files processed: " << stats.html_files_processed << "\n\n";

    // Test 6: Quality Assessment Analysis
    std::cout << "6. Quality Assessment Analysis\n";
    std::cout << "-----------------------------\n";
    
    std::cout << "Quality filtering configuration:\n";
    std::cout << "  Min content quality score: 0.3\n";
    std::cout << "  Min information density: 0.1\n";
    std::cout << "  Min content length: 50 characters\n";
    std::cout << "  Max content length: 1,000,000 characters\n";
    std::cout << "  Filter empty documents: YES\n";
    std::cout << "  Filter low-quality documents: YES\n\n";
    
    std::cout << "Quality assessment metrics:\n";
    std::cout << "  Content quality score: Based on length, word diversity, sentence structure, and information density\n";
    std::cout << "  Information density: Based on unique word ratio, technical terms, and sentence complexity\n";
    std::cout << "  High-quality threshold: Meets all minimum requirements\n\n";

    std::cout << "Advanced features demonstrated:\n";
    std::cout << "  ✅ Quality assessment and scoring\n";
    std::cout << "  ✅ Information density calculation\n";
    std::cout << "  ✅ Content filtering based on quality\n";
    std::cout << "  ✅ Batch processing with filtering\n";
    std::cout << "  ✅ Statistics tracking for quality metrics\n";
    std::cout << "  ✅ Configuration-driven quality settings\n";

    return 0;
} 