#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
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
    processor.initialize(config);

    std::ofstream results_file("../data/test_results.txt");
    results_file << "R3M Comprehensive Test Results\n";
    results_file << "==============================\n\n";

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

    // Test 4: Processing Statistics
    results_file << "\n4. PROCESSING STATISTICS\n";
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

    // Summary
    results_file << "\n5. SUMMARY\n";
    results_file << "==========\n";
    results_file << "Total formats tested: " << (text_formats.size() + 3) << "\n";
    results_file << "Text formats: " << text_formats.size() << "\n";
    results_file << "HTML formats: 2 (simple + complex)\n";
    results_file << "PDF format: 1\n";
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

    return 0;
}