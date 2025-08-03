#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <iomanip>
#include <unordered_map>
#include <chrono>
#include "r3m/core/document_processor.hpp"
#include "r3m/chunking/chunk_models.hpp"

using namespace r3m;

void create_large_test_documents() {
    std::filesystem::create_directories("data");
    
    // Create a large technical document
    std::ofstream large_tech("data/large_technical_document.txt");
    large_tech << "R3M Advanced Document Processing System\n";
    large_tech << "========================================\n\n";
    
    // Add multiple sections with proper sentences
    for (int section = 1; section <= 20; ++section) {
        large_tech << "SECTION " << section << ": ADVANCED PROCESSING FEATURES\n";
        large_tech << "==============================================\n\n";
        
        large_tech << "The R3M system provides comprehensive document processing capabilities that enable organizations ";
        large_tech << "to efficiently handle large volumes of diverse content. This section explores the advanced ";
        large_tech << "features that make R3M a powerful solution for modern document management needs.\n\n";
        
        large_tech << "Parallel processing capabilities allow the system to handle multiple documents simultaneously, ";
        large_tech << "dramatically improving throughput and reducing processing time. The implementation uses ";
        large_tech << "custom thread pools that can be configured based on system resources and workload requirements.\n\n";
        
        large_tech << "Quality assessment algorithms analyze content characteristics including information density, ";
        large_tech << "technical term frequency, and semantic coherence. These metrics help identify high-value ";
        large_tech << "content and filter out low-quality or irrelevant documents automatically.\n\n";
        
        large_tech << "Chunking algorithms preserve semantic boundaries while ensuring optimal token distribution. ";
        large_tech << "The system maintains sentence integrity and paragraph structure while creating chunks that ";
        large_tech << "meet specified size and quality requirements.\n\n";
        
        large_tech << "Multi-format support enables processing of various document types including PDF files, ";
        large_tech << "HTML documents, Markdown files, and plain text. Each format is handled with appropriate ";
        large_tech << "parsing and extraction techniques to maximize content quality.\n\n";
        
        large_tech << "The modular architecture allows for easy extension and customization. Components can be ";
        large_tech << "replaced or enhanced without affecting the overall system functionality. This design ";
        large_tech << "promotes maintainability and enables future enhancements.\n\n";
        
        large_tech << "Performance optimization techniques include memory-efficient processing, intelligent caching, ";
        large_tech << "and adaptive resource allocation. These features ensure consistent performance even with ";
        large_tech << "large document collections and varying system loads.\n\n";
        
        large_tech << "Integration capabilities allow R3M to work seamlessly with existing document management ";
        large_tech << "systems and workflows. The system provides APIs and interfaces that enable easy ";
        large_tech << "integration with enterprise applications and custom solutions.\n\n";
        
        large_tech << "Security features include secure document handling, access control mechanisms, and audit ";
        large_tech << "trails for compliance requirements. These features ensure that sensitive documents are ";
        large_tech << "processed and stored according to organizational security policies.\n\n";
        
        large_tech << "Scalability is achieved through horizontal scaling capabilities and efficient resource ";
        large_tech << "utilization. The system can handle increasing workloads by adding processing nodes ";
        large_tech << "and distributing tasks across multiple instances.\n\n";
    }
    large_tech.close();
    
    // Create a large narrative document
    std::ofstream large_narrative("data/large_narrative_document.txt");
    large_narrative << "The Evolution of Document Processing Technology\n";
    large_narrative << "==============================================\n\n";
    
    for (int chapter = 1; chapter <= 15; ++chapter) {
        large_narrative << "CHAPTER " << chapter << ": HISTORICAL PERSPECTIVES\n";
        large_narrative << "==========================================\n\n";
        
        large_narrative << "The journey of document processing technology began in the early days of computing, ";
        large_narrative << "when simple text processing tools were developed to handle basic formatting tasks. ";
        large_narrative << "These early systems laid the foundation for more sophisticated document management ";
        large_narrative << "solutions that would emerge in subsequent decades.\n\n";
        
        large_narrative << "As computing power increased and storage capabilities expanded, document processing ";
        large_narrative << "systems evolved to handle larger volumes of information. The introduction of ";
        large_narrative << "relational databases and structured data formats enabled more efficient storage ";
        large_narrative << "and retrieval of document content.\n\n";
        
        large_narrative << "The advent of the internet and web technologies brought new challenges and ";
        large_narrative << "opportunities for document processing. HTML documents, PDF files, and various ";
        large_narrative << "digital formats required new parsing and processing techniques that could ";
        large_narrative << "maintain content integrity while enabling efficient processing.\n\n";
        
        large_narrative << "Machine learning and artificial intelligence technologies have revolutionized ";
        large_narrative << "document processing capabilities. Natural language processing algorithms can ";
        large_narrative << "now understand context, extract meaning, and provide intelligent insights ";
        large_narrative << "from document content.\n\n";
        
        large_narrative << "Cloud computing has enabled scalable document processing solutions that can ";
        large_narrative << "handle massive volumes of documents across distributed systems. This approach ";
        large_narrative << "provides flexibility, reliability, and cost-effectiveness for organizations ";
        large_narrative << "of all sizes.\n\n";
        
        large_narrative << "Modern document processing systems must address challenges related to data ";
        large_narrative << "privacy, security, and compliance requirements. Advanced encryption, access ";
        large_narrative << "controls, and audit capabilities ensure that sensitive information is ";
        large_narrative << "protected throughout the processing pipeline.\n\n";
        
        large_narrative << "The future of document processing lies in intelligent automation and ";
        large_narrative << "seamless integration with business workflows. Systems that can understand ";
        large_narrative << "context, learn from user interactions, and adapt to changing requirements ";
        large_narrative << "will provide the greatest value to organizations.\n\n";
        
        large_narrative << "Collaboration features enable teams to work together on document processing ";
        large_narrative << "tasks, sharing insights and improving efficiency through collective ";
        large_narrative << "intelligence. Real-time collaboration tools facilitate communication ";
        large_narrative << "and coordination among team members.\n\n";
        
        large_narrative << "Mobile computing has created new requirements for document processing ";
        large_narrative << "systems. Responsive design, touch interfaces, and offline capabilities ";
        large_narrative << "enable users to process documents on various devices and platforms ";
        large_narrative << "seamlessly.\n\n";
        
        large_narrative << "Integration with enterprise systems and third-party applications ";
        large_narrative << "ensures that document processing capabilities are available where ";
        large_narrative << "they are needed most. APIs and standard interfaces enable seamless ";
        large_narrative << "connectivity with existing business processes.\n\n";
    }
    large_narrative.close();
    
    // Create a large mixed-content document
    std::ofstream large_mixed("data/large_mixed_content.txt");
    large_mixed << "Comprehensive Analysis Report: Document Processing Systems\n";
    large_mixed << "========================================================\n\n";
    
    for (int section = 1; section <= 25; ++section) {
        large_mixed << "ANALYSIS SECTION " << section << ": SYSTEM EVALUATION\n";
        large_mixed << "==============================================\n\n";
        
        large_mixed << "Performance metrics indicate that modern document processing systems achieve ";
        large_mixed << "significant improvements in processing speed and accuracy compared to traditional ";
        large_mixed << "methods. Benchmark tests show processing times reduced by 60-80% while maintaining ";
        large_mixed << "or improving content quality scores.\n\n";
        
        large_mixed << "Quality assessment algorithms evaluate multiple dimensions of document content ";
        large_mixed << "including readability, information density, and semantic coherence. These ";
        large_mixed << "metrics help identify high-value content and filter out low-quality documents ";
        large_mixed << "automatically, improving overall system efficiency.\n\n";
        
        large_mixed << "Chunking strategies preserve semantic boundaries while optimizing for processing ";
        large_mixed << "efficiency. Advanced algorithms maintain sentence and paragraph integrity while ";
        large_mixed << "creating chunks that meet size and quality requirements. This approach ensures ";
        large_mixed << "that downstream processing maintains context and meaning.\n\n";
        
        large_mixed << "Scalability testing demonstrates that the system can handle document volumes ";
        large_mixed << "ranging from thousands to millions of files with consistent performance. ";
        large_mixed << "Horizontal scaling capabilities allow organizations to add processing capacity ";
        large_mixed << "as needed without significant architectural changes.\n\n";
        
        large_mixed << "Security analysis reveals robust protection mechanisms for sensitive document ";
        large_mixed << "content. Encryption at rest and in transit, access control systems, and ";
        large_mixed << "comprehensive audit trails ensure compliance with regulatory requirements ";
        large_mixed << "while protecting organizational data assets.\n\n";
        
        large_mixed << "Integration capabilities enable seamless connectivity with existing enterprise ";
        large_mixed << "systems and workflows. Standard APIs and interfaces facilitate deployment ";
        large_mixed << "across diverse technology environments while maintaining operational ";
        large_mixed << "efficiency and user productivity.\n\n";
        
        large_mixed << "Cost-benefit analysis shows significant return on investment for organizations ";
        large_mixed << "implementing advanced document processing solutions. Reduced manual effort, ";
        large_mixed << "improved accuracy, and faster processing times contribute to measurable ";
        large_mixed << "business value and competitive advantages.\n\n";
        
        large_mixed << "User experience evaluations indicate high satisfaction with intuitive interfaces ";
        large_mixed << "and responsive system performance. Training requirements are minimal due to ";
        large_mixed << "well-designed workflows and comprehensive documentation that supports ";
        large_mixed << "effective system utilization.\n\n";
        
        large_mixed << "Maintenance and support requirements are minimized through modular architecture ";
        large_mixed << "and comprehensive monitoring capabilities. Proactive maintenance features ";
        large_mixed << "identify potential issues before they impact system performance, ensuring ";
        large_mixed << "reliable operation and minimal downtime.\n\n";
        
        large_mixed << "Future development plans include enhanced machine learning capabilities, ";
        large_mixed << "improved natural language processing, and expanded format support. These ";
        large_mixed << "advancements will further improve processing accuracy and enable new ";
        large_mixed << "applications for document analysis and insights.\n\n";
    }
    large_mixed.close();
    
    std::cout << "✅ Created large test documents:\n";
    std::cout << "  - large_technical_document.txt\n";
    std::cout << "  - large_narrative_document.txt\n";
    std::cout << "  - large_mixed_content.txt\n\n";
}

void analyze_chunk_quality(const std::vector<chunking::DocumentChunk>& chunks, const std::string& filename) {
    std::cout << "📊 CHUNK QUALITY ANALYSIS: " << filename << "\n";
    std::cout << "==========================================\n";
    
    size_t total_chunks = chunks.size();
    double total_quality = 0.0;
    double total_density = 0.0;
    size_t sentences_preserved = 0;
    size_t chunks_with_technical_terms = 0;
    
    for (size_t i = 0; i < chunks.size(); ++i) {
        const auto& chunk = chunks[i];
        total_quality += chunk.quality_score;
        total_density += chunk.information_density;
        
        // Check for sentence preservation (look for proper sentence endings)
        std::string content = chunk.content;
        if (content.find('.') != std::string::npos || 
            content.find('!') != std::string::npos || 
            content.find('?') != std::string::npos) {
            sentences_preserved++;
        }
        
        // Check for technical terms
        if (content.find("algorithm") != std::string::npos ||
            content.find("system") != std::string::npos ||
            content.find("processing") != std::string::npos ||
            content.find("technology") != std::string::npos ||
            content.find("document") != std::string::npos) {
            chunks_with_technical_terms++;
        }
        
        std::cout << "  Chunk " << i << ":\n";
        std::cout << "    Length: " << chunk.content.length() << " chars\n";
        std::cout << "    Quality: " << chunk.quality_score << "\n";
        std::cout << "    Density: " << chunk.information_density << "\n";
        std::cout << "    Tokens: " << chunk.title_tokens + chunk.metadata_tokens << "\n";
        
        // Show first 100 characters of content
        std::string preview = chunk.content.substr(0, 100);
        if (chunk.content.length() > 100) preview += "...";
        std::cout << "    Preview: " << preview << "\n\n";
    }
    
    std::cout << "📈 SUMMARY STATISTICS:\n";
    std::cout << "  Total chunks: " << total_chunks << "\n";
    std::cout << "  Average quality: " << (total_quality / total_chunks) << "\n";
    std::cout << "  Average density: " << (total_density / total_chunks) << "\n";
    std::cout << "  Sentences preserved: " << sentences_preserved << "/" << total_chunks << "\n";
    std::cout << "  Chunks with technical terms: " << chunks_with_technical_terms << "/" << total_chunks << "\n";
    std::cout << "\n";
}

int main() {
    std::cout << "R3M LARGE DOCUMENT CHUNKING TEST\n";
    std::cout << "=================================\n\n";
    
    // Create large test documents
    std::cout << "Creating large test documents...\n";
    create_large_test_documents();
    
    // Initialize processor
    std::unordered_map<std::string, std::string> config = {
        {"document_processing.enable_chunking", "true"},
        {"chunking.enable_multipass", "true"},
        {"chunking.enable_large_chunks", "true"},
        {"chunking.enable_contextual_rag", "true"},
        {"chunking.chunk_token_limit", "1000"},
        {"chunking.max_metadata_percentage", "0.1"},
        {"chunking.quality_threshold", "0.3"},
        {"chunking.density_threshold", "0.5"}
    };
    
    core::DocumentProcessor processor;
    processor.initialize(config);
    
    std::cout << "✅ Processor initialized\n\n";
    
    // Test files
    std::vector<std::string> test_files = {
        "data/large_technical_document.txt",
        "data/large_narrative_document.txt",
        "data/large_mixed_content.txt"
    };
    
    for (const auto& file : test_files) {
        std::cout << "🔍 Processing: " << file << "\n";
        
        // Check file size
        std::ifstream check_file(file);
        check_file.seekg(0, std::ios::end);
        size_t file_size = check_file.tellg();
        check_file.close();
        
        std::cout << "  📄 File size: " << file_size << " bytes (" << (file_size / 1024.0) << " KB)\n";
        
        // Time the processing
        auto start_time = std::chrono::high_resolution_clock::now();
        auto chunking_result = processor.process_document_with_chunking(file);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto processing_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "  ⏱️  Processing time: " << processing_time.count() << " ms\n";
        std::cout << "  📊 Chunking results:\n";
        std::cout << "    Total chunks: " << chunking_result.total_chunks << "\n";
        std::cout << "    Successful chunks: " << chunking_result.successful_chunks << "\n";
        std::cout << "    Average quality: " << chunking_result.avg_quality_score << "\n";
        std::cout << "    Average density: " << chunking_result.avg_information_density << "\n";
        
        // Calculate processing speed
        double speed_kb_per_sec = (file_size / 1024.0) / (processing_time.count() / 1000.0);
        std::cout << "    Processing speed: " << speed_kb_per_sec << " KB/s\n";
        
        // Analyze chunk quality
        analyze_chunk_quality(chunking_result.chunks, file);
        
        std::cout << "\n";
    }
    
    std::cout << "✅ Large document chunking test complete!\n";
    std::cout << "📊 Check the results above for performance and quality metrics.\n";
    
    return 0;
} 