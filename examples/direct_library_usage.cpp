#include "r3m/core/library.hpp"
#include <iostream>
#include <vector>

int main() {
    std::cout << "🚀 R3M Direct Library Usage Example" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // Initialize R3M Library (zero overhead)
    r3m::core::Library library;
    
    if (!library.initialize("configs/dev/config.yaml")) {
        std::cerr << "❌ Failed to initialize R3M Library" << std::endl;
        return 1;
    }
    
    std::cout << "✅ R3M Library initialized successfully" << std::endl;
    
    // Example 1: Single document processing (zero overhead)
    std::cout << "\n📄 Processing single document..." << std::endl;
    try {
        auto result = library.process_document("data/test_document.txt");
        
        std::cout << "File: " << result.file_name << std::endl;
        std::cout << "Success: " << (result.processing_success ? "YES" : "NO") << std::endl;
        std::cout << "Processing time: " << result.processing_time_ms << " ms" << std::endl;
        std::cout << "Text length: " << result.text_content.length() << " characters" << std::endl;
        std::cout << "Quality score: " << result.content_quality_score << std::endl;
        std::cout << "Information density: " << result.information_density << std::endl;
        std::cout << "High quality: " << (result.is_high_quality ? "YES" : "NO") << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error processing document: " << e.what() << std::endl;
    }
    
    // Example 2: Parallel batch processing (zero overhead)
    std::cout << "\n📚 Processing multiple documents in parallel..." << std::endl;
    try {
        std::vector<std::string> files = {
            "data/test_document.txt",
            "data/test_document.md",
            "data/test_document.json"
        };
        
        auto results = library.process_documents_parallel(files);
        
        std::cout << "Processed " << results.size() << " files:" << std::endl;
        for (const auto& result : results) {
            std::cout << "  " << result.file_name << ": " 
                      << result.text_content.length() << " chars, "
                      << "quality: " << result.content_quality_score << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error processing batch: " << e.what() << std::endl;
    }
    
    // Example 3: Batch processing with quality filtering
    std::cout << "\n🔍 Batch processing with quality filtering..." << std::endl;
    try {
        std::vector<std::string> files = {
            "data/high_quality.txt",
            "data/medium_quality.txt",
            "data/low_quality.txt",
            "data/empty.txt"
        };
        
        auto batch_result = library.process_batch_with_filtering(files);
        
        std::cout << "Batch processing results:" << std::endl;
        std::cout << "  Total files: " << batch_result.total_files << std::endl;
        std::cout << "  Processed: " << batch_result.processed.size() << std::endl;
        std::cout << "  Filtered out: " << batch_result.filtered_out.size() << std::endl;
        std::cout << "  Processing time: " << batch_result.processing_time_ms << " ms" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error in batch processing: " << e.what() << std::endl;
    }
    
    // Example 4: Get statistics
    std::cout << "\n📊 Library Statistics:" << std::endl;
    try {
        auto stats = library.get_statistics();
        
        std::cout << "  Total files processed: " << stats.total_files_processed << std::endl;
        std::cout << "  Successful processing: " << stats.successful_processing << std::endl;
        std::cout << "  Failed processing: " << stats.failed_processing << std::endl;
        std::cout << "  Average processing time: " << stats.avg_processing_time_ms << " ms" << std::endl;
        std::cout << "  Total text extracted: " << stats.total_text_extracted << " characters" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error getting statistics: " << e.what() << std::endl;
    }
    
    std::cout << "\n✅ Direct library usage example completed!" << std::endl;
    std::cout << "💡 This approach has ZERO API overhead - direct C++ function calls!" << std::endl;
    
    return 0;
} 