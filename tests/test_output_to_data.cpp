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

    // Set up processor
    DocumentProcessor processor;
    std::unordered_map<std::string, std::string> config;
    config["document_processing.max_file_size"] = "100MB";
    config["document_processing.max_text_length"] = "1000000";
    config["document_processing.text_processing.encoding_detection"] = "true";
    config["document_processing.text_processing.default_encoding"] = "utf-8";
    config["document_processing.text_processing.remove_html_tags"] = "true";
    config["document_processing.text_processing.normalize_whitespace"] = "true";
    config["document_processing.text_processing.extract_metadata"] = "true";
    config["document_processing.batch_size"] = "8";  // Smaller batch for testing
    config["document_processing.max_workers"] = "4"; // 4 worker threads
    processor.initialize(config);

    std::ofstream results_file("../data/test_results.txt");
    results_file << "R3M Comprehensive Test Results - Parallel & Batch Processing\n";
    results_file << "============================================================\n\n";

    // Test 1: Plain Text Files (all supported formats)
    std::vector<std::string> text_formats = {
        ".txt", ".md", ".mdx", ".conf", ".log", ".json", ".csv", ".tsv", ".xml", ".yml", ".yaml"
    };

    results_file << "1. PLAIN TEXT FORMATS TESTING\n";
    results_file << "=============================\n";

    for (const auto& format : text_formats) {
        std::string filename = "test_document" + format;
        std::string filepath = "../data/" + filename;
        
        // Create test file
        std::ofstream test_file(filepath);
        test_file << "This is a test " << format << " file for R3M.\n";
        test_file << "Testing format: " << format << "\n";
        test_file << "Content should be extracted properly.\n";
        test_file.close();

        // Process the file
        DocumentResult result = processor.process_document(filepath);
        
        results_file << "\nFormat: " << format << "\n";
        results_file << "File: " << filename << "\n";
        results_file << "Success: " << (result.processing_success ? "YES" : "NO") << "\n";
        results_file << "Processing time: " << result.processing_time_ms << " ms\n";
        results_file << "Text length: " << result.text_content.length() << " characters\n";
        results_file << "Extracted text: " << result.text_content << "\n";
        results_file << "---\n";
    }

    // Test 2: HTML Files (multiple test cases)
    results_file << "\n2. HTML FORMAT TESTING\n";
    results_file << "======================\n";

    // HTML Test Case 1: Simple HTML
    std::string html_content_1 = R"(
<!DOCTYPE html>
<html>
<head>
    <title>R3M HTML Test</title>
    <style>body { font-family: Arial; }</style>
</head>
<body>
    <h1>R3M HTML Processing Test</h1>
    <p>This is a test HTML document for R3M document processing.</p>
    <div>
        <h2>Features to Test:</h2>
        <ul>
            <li>Text extraction from HTML</li>
            <li>Removal of HTML tags</li>
            <li>Preservation of text content</li>
        </ul>
    </div>
    <script>console.log("This should be ignored");</script>
</body>
</html>
)";

    std::string html_filepath_1 = "../data/test_document.html";
    std::ofstream html_file_1(html_filepath_1);
    html_file_1 << html_content_1;
    html_file_1.close();

    DocumentResult html_result_1 = processor.process_document(html_filepath_1);
    
    results_file << "\nHTML Test Case 1 (Simple HTML):\n";
    results_file << "Success: " << (html_result_1.processing_success ? "YES" : "NO") << "\n";
    results_file << "Processing time: " << html_result_1.processing_time_ms << " ms\n";
    results_file << "Text length: " << html_result_1.text_content.length() << " characters\n";
    results_file << "Extracted text: " << html_result_1.text_content << "\n";
    results_file << "---\n";

    // HTML Test Case 2: Complex HTML with more content
    std::string html_content_2 = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>R3M Complex HTML Test</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .header { background-color: #f0f0f0; padding: 10px; }
        .content { margin: 20px 0; }
        .footer { border-top: 1px solid #ccc; padding-top: 10px; }
    </style>
</head>
<body>
    <div class="header">
        <h1>R3M Advanced HTML Processing Test</h1>
        <p>Testing complex HTML structure with various elements</p>
    </div>
    
    <div class="content">
        <h2>Document Structure Testing</h2>
        <p>This document contains multiple sections to test HTML processing capabilities.</p>
        
        <h3>Text Elements</h3>
        <p>Testing various text elements including <strong>bold text</strong>, <em>italic text</em>, and <code>code snippets</code>.</p>
        
        <h3>Lists and Tables</h3>
        <ul>
            <li>Unordered list item 1</li>
            <li>Unordered list item 2</li>
            <li>Unordered list item 3</li>
        </ul>
        
        <ol>
            <li>Ordered list item 1</li>
            <li>Ordered list item 2</li>
            <li>Ordered list item 3</li>
        </ol>
        
        <table border="1">
            <tr><th>Header 1</th><th>Header 2</th></tr>
            <tr><td>Data 1</td><td>Data 2</td></tr>
            <tr><td>Data 3</td><td>Data 4</td></tr>
        </table>
    </div>
    
    <div class="footer">
        <p>Footer content with additional information.</p>
        <p>This should be extracted as plain text.</p>
    </div>
    
    <script>
        // This script content should be ignored
        console.log("JavaScript should be ignored");
        function testFunction() {
            return "This should not appear in extracted text";
        }
    </script>
    
    <style>
        /* CSS should also be ignored */
        .ignored { display: none; }
    </style>
</body>
</html>
)";

    std::string html_filepath_2 = "../data/test_document_complex.html";
    std::ofstream html_file_2(html_filepath_2);
    html_file_2 << html_content_2;
    html_file_2.close();

    DocumentResult html_result_2 = processor.process_document(html_filepath_2);
    
    results_file << "\nHTML Test Case 2 (Complex HTML):\n";
    results_file << "Success: " << (html_result_2.processing_success ? "YES" : "NO") << "\n";
    results_file << "Processing time: " << html_result_2.processing_time_ms << " ms\n";
    results_file << "Text length: " << html_result_2.text_content.length() << " characters\n";
    results_file << "Extracted text: " << html_result_2.text_content << "\n";
    results_file << "---\n";

    // Test 3: PDF File
    results_file << "\n3. PDF FORMAT TESTING\n";
    results_file << "=====================\n";

    // Create HTML content for PDF conversion
    std::string pdf_html_content = R"(
<!DOCTYPE html>
<html>
<head>
    <title>R3M PDF Test Document</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        p { line-height: 1.6; }
        .highlight { background-color: #f0f0f0; padding: 10px; }
    </style>
</head>
<body>
    <h1>R3M PDF Processing Test</h1>
    
    <p>This is a test PDF document for R3M document processing.</p>
    
    <p>The PDF processor should extract this text content successfully from the PDF file.</p>
    
    <div class="highlight">
        <h2>Features to Test:</h2>
        <ul>
            <li>Text extraction from PDF documents</li>
            <li>Multi-page PDF processing</li>
            <li>Preservation of text content</li>
            <li>Fast processing performance</li>
        </ul>
    </div>
    
    <p>This document contains various text elements including:</p>
    <ul>
        <li>Headings and subheadings</li>
        <li>Paragraphs with different formatting</li>
        <li>Lists and bullet points</li>
        <li>Styled content with CSS</li>
    </ul>
    
    <p>The R3M PDF processor uses poppler-cpp library to extract text content from PDF files efficiently and accurately.</p>
</body>
</html>
)";

    // Create HTML file for PDF conversion
    std::string pdf_html_filepath = "../data/temp_pdf_content.html";
    std::ofstream pdf_html_file(pdf_html_filepath);
    pdf_html_file << pdf_html_content;
    pdf_html_file.close();

    // Convert HTML to PDF using weasyprint
    std::string pdf_filepath = "../data/test_document.pdf";
    std::string convert_cmd = "weasyprint " + pdf_html_filepath + " " + pdf_filepath;
    system(convert_cmd.c_str());

    // Check if PDF was created successfully
    if (std::filesystem::exists(pdf_filepath)) {
        DocumentResult pdf_result = processor.process_document(pdf_filepath);
        
        results_file << "\nPDF Processing Results:\n";
        results_file << "Success: " << (pdf_result.processing_success ? "YES" : "NO") << "\n";
        results_file << "Processing time: " << pdf_result.processing_time_ms << " ms\n";
        results_file << "Text length: " << pdf_result.text_content.length() << " characters\n";
        results_file << "Extracted text: " << pdf_result.text_content << "\n";
        results_file << "---\n";
    } else {
        results_file << "\nPDF Processing Results:\n";
        results_file << "Error: PDF file could not be created\n";
        results_file << "---\n";
    }

    // Clean up temporary HTML file
    std::filesystem::remove(pdf_html_filepath);

    // Test 4: Parallel Processing Test
    results_file << "\n4. PARALLEL PROCESSING TEST\n";
    results_file << "==========================\n";

    // Create multiple test files for parallel processing
    std::vector<std::string> parallel_test_files;
    for (int i = 0; i < 10; ++i) {
        std::string filename = "parallel_test_" + std::to_string(i) + ".txt";
        std::string filepath = "../data/" + filename;
        
        std::ofstream test_file(filepath);
        test_file << "This is parallel test file " << i << " for R3M.\n";
        test_file << "Testing parallel processing capabilities.\n";
        test_file << "File number: " << i << "\n";
        test_file.close();
        
        parallel_test_files.push_back(filepath);
    }

    // Test parallel processing
    auto parallel_start = std::chrono::high_resolution_clock::now();
    auto parallel_results = processor.process_documents_parallel(parallel_test_files);
    auto parallel_end = std::chrono::high_resolution_clock::now();
    
    auto parallel_duration = std::chrono::duration_cast<std::chrono::milliseconds>(parallel_end - parallel_start);
    
    results_file << "\nParallel Processing Results:\n";
    results_file << "Files processed: " << parallel_results.size() << "\n";
    results_file << "Total processing time: " << parallel_duration.count() << " ms\n";
    results_file << "Average time per file: " << (parallel_duration.count() > 0 ? parallel_duration.count() / parallel_results.size() : 0) << " ms\n";
    
    int successful_parallel = 0;
    for (const auto& result : parallel_results) {
        if (result.processing_success) {
            successful_parallel++;
        }
    }
    results_file << "Successful processing: " << successful_parallel << "/" << parallel_results.size() << "\n";
    results_file << "---\n";

    // Test 5: Batch Processing Test
    results_file << "\n5. BATCH PROCESSING TEST\n";
    results_file << "========================\n";

    // Create more test files for batch processing
    std::vector<std::string> batch_test_files;
    for (int i = 0; i < 20; ++i) {
        std::string filename = "batch_test_" + std::to_string(i) + ".txt";
        std::string filepath = "../data/" + filename;
        
        std::ofstream test_file(filepath);
        test_file << "This is batch test file " << i << " for R3M.\n";
        test_file << "Testing batch processing capabilities.\n";
        test_file << "Batch file number: " << i << "\n";
        test_file.close();
        
        batch_test_files.push_back(filepath);
    }

    // Test batch processing
    auto batch_start = std::chrono::high_resolution_clock::now();
    auto batch_results = processor.process_documents_batch(batch_test_files);
    auto batch_end = std::chrono::high_resolution_clock::now();
    
    auto batch_duration = std::chrono::duration_cast<std::chrono::milliseconds>(batch_end - batch_start);
    
    results_file << "\nBatch Processing Results:\n";
    results_file << "Files processed: " << batch_results.size() << "\n";
    results_file << "Total processing time: " << batch_duration.count() << " ms\n";
    results_file << "Average time per file: " << (batch_duration.count() > 0 ? batch_duration.count() / batch_results.size() : 0) << " ms\n";
    
    int successful_batch = 0;
    for (const auto& result : batch_results) {
        if (result.processing_success) {
            successful_batch++;
        }
    }
    results_file << "Successful processing: " << successful_batch << "/" << batch_results.size() << "\n";
    results_file << "---\n";

    // Test 6: Processing Statistics
    results_file << "\n6. PROCESSING STATISTICS\n";
    results_file << "========================\n";
    
    auto stats = processor.get_processing_stats();
    results_file << "Total files processed: " << stats.total_files_processed << "\n";
    results_file << "Successful processing: " << stats.successful_processing << "\n";
    results_file << "Failed processing: " << stats.failed_processing << "\n";
    results_file << "Average processing time: " << stats.avg_processing_time_ms << " ms\n";
    results_file << "Total text extracted: " << stats.total_text_extracted << " characters\n";
    results_file << "Text files processed: " << stats.text_files_processed << "\n";
    results_file << "PDF files processed: " << stats.pdf_files_processed << "\n";
    results_file << "HTML files processed: " << stats.html_files_processed << "\n";

    // Performance comparison
    results_file << "\n7. PERFORMANCE COMPARISON\n";
    results_file << "=========================\n";
    results_file << "Parallel processing time: " << parallel_duration.count() << " ms for " << parallel_results.size() << " files\n";
    results_file << "Batch processing time: " << batch_duration.count() << " ms for " << batch_results.size() << " files\n";
    
    double parallel_throughput = (parallel_duration.count() > 0) ? 
        (parallel_results.size() * 1000.0 / parallel_duration.count()) : 0;
    double batch_throughput = (batch_duration.count() > 0) ? 
        (batch_results.size() * 1000.0 / batch_duration.count()) : 0;
    
    results_file << "Parallel throughput: " << parallel_throughput << " files/second\n";
    results_file << "Batch throughput: " << batch_throughput << " files/second\n";

    // Summary
    results_file << "\n8. SUMMARY\n";
    results_file << "==========\n";
    results_file << "Total formats tested: " << (text_formats.size() + 3) << "\n";
    results_file << "Text formats: " << text_formats.size() << "\n";
    results_file << "HTML formats: 2 (simple + complex)\n";
    results_file << "PDF format: 1\n";
    results_file << "Parallel processing: " << parallel_results.size() << " files\n";
    results_file << "Batch processing: " << batch_results.size() << " files\n";
    results_file << "\nAll test files created in: ../data/\n";
    results_file << "Main result file: ../data/test_results.txt\n";

    results_file.close();

    std::cout << "Comprehensive testing completed!\n";
    std::cout << "Results written to: ../data/test_results.txt\n";
    std::cout << "All test files created in: ../data/\n";
    std::cout << "Processing statistics:\n";
    std::cout << "  Total files processed: " << stats.total_files_processed << "\n";
    std::cout << "  Text files: " << stats.text_files_processed << "\n";
    std::cout << "  HTML files: " << stats.html_files_processed << "\n";
    std::cout << "  PDF files: " << stats.pdf_files_processed << "\n";
    std::cout << "  Parallel processing: " << parallel_results.size() << " files\n";
    std::cout << "  Batch processing: " << batch_results.size() << " files\n";

    return 0;
}