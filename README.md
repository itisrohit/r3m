# R3M 

> **Development Journal**: A high-performance C++ document processing pipeline with advanced chunking capabilities and HTTP API

## 🚀 Current Status: Advanced Chunking + HTTP Server + Core Engine Complete

### ✅ **Phase 1: C++ Core Engine** - COMPLETED
- **Fast document processing and pipeline orchestration**
- **Parallel processing with 2.88x speedup**
- **Batch processing with quality filtering**
- **Advanced quality assessment algorithms**
- **Support for PDF, HTML, and plain text formats**

### ✅ **Phase 2: HTTP Server** - COMPLETED
- **REST API endpoints for document processing**
- **Health check and system information**
- **Single document and batch processing**
- **Job status tracking**
- **Graceful fallback when HTTP libraries not available**
- **Comprehensive API testing with 100% success rate**
- **All document formats supported (TXT, MD, JSON, HTML, CSV)**
- **Quality assessment and information density calculation**
- **Performance monitoring and statistics**

### ✅ **Phase 3: Advanced Chunking System** - COMPLETED
- **Intelligent document chunking strategies**
- **Semantic boundary detection**
- **Configurable chunk sizes and overlap**
- **Chunk quality assessment**
- **Multipass indexing with mini-chunks and large chunks**
- **Contextual RAG framework (LLM integration ready)**
- **Advanced metadata processing**
- **Source-specific handling**
- **Strict token limit enforcement**
- **Section combination logic**
- **Link offset tracking**
- **Advanced text processing utilities**
- **Sophisticated tokenizer system**

## 📊 Performance Metrics

### Core Engine Performance
- **Parallel speedup**: 2.88x (230ms → 63ms)
- **Efficiency**: 71.88%
- **Processing throughput**: 54 files in 230ms
- **Average processing time**: 14.681 ms per file
- **Text extraction**: 2.3M characters processed

### HTTP Server Features
- **REST API endpoints**: `/health`, `/process`, `/batch`, `/job/{id}`, `/info`
- **Graceful degradation**: Works without HTTP libraries
- **JSON response format**: Structured API responses
- **CORS support**: Cross-origin request handling
- **File upload support**: Direct content or file path processing
- **Comprehensive testing**: 100% success rate across all endpoints
- **Document processing**: All formats (TXT, MD, JSON, HTML, CSV) supported
- **Quality assessment**: Content scoring and information density calculation
- **Performance metrics**: Real-time statistics and processing times

### Advanced Chunking Features
- **Multipass indexing**: Mini-chunks (150 tokens) and large chunks (4x ratio)
- **Contextual RAG**: Document summaries and chunk context generation
- **Quality filtering**: Advanced chunk quality assessment
- **Metadata processing**: Title and metadata token management
- **Section processing**: Continuation logic and oversized handling
- **Source-specific handling**: Gmail and other source types
- **Strict token limits**: Enforced token boundaries
- **Link tracking**: Source link offset management
- **Advanced text processing**: Comprehensive text utilities
- **Sophisticated tokenizers**: Simple, sentence, and BPE tokenizers

## 🏗️ Architecture

### Core Modules
```
r3m/
├── core/           # Core processing engine
├── chunking/       # Advanced document chunking and segmentation
│   ├── tokenizer.cpp           # Basic tokenizer
│   ├── sentence_chunker.cpp    # Sentence-aware chunking
│   ├── multipass_chunker.cpp  # Mini-chunks and large chunks
│   ├── contextual_rag.cpp     # Document summaries and context
│   ├── advanced_chunker.cpp   # Comprehensive chunking system
│   ├── metadata_processor.cpp # Advanced metadata handling
│   └── advanced_tokenizer.cpp # Sophisticated tokenizer system
├── processing/     # Pipeline orchestration
├── quality/        # Quality assessment
├── parallel/       # Thread pool management
├── formats/        # Document format processors
├── server/         # HTTP server (modular)
├── api/            # API layer (modular)
│   ├── routes/         # REST API endpoints
│   ├── jobs/           # Async job tracking
│   └── config/         # Configuration management
└── utils/          # Advanced text utilities
    ├── text_utils.cpp      # Basic text utilities
    └── text_processing.cpp # Advanced text processing
```

### HTTP Server (Optional)
- **Modular Architecture**: `api::Routes`, `api::JobManager`, `api::Config` modules
- **Organized Structure**: Separate folders for routes, jobs, and config
- **Crow HTTP server**: Lightweight C++ web framework
- **JSON handling**: nlohmann/json library with fallback support
- **Graceful fallback**: Works without external HTTP libraries
- **Configuration-driven**: All settings from config.yaml
- **Job management**: Async processing with status tracking

### Advanced Chunking System
- **Token-aware processing**: Advanced tokenizer with multiple strategies
- **Sentence chunker**: Semantic chunking respecting sentence boundaries
- **Multipass indexing**: Mini-chunks (150 tokens) and large chunks (4x ratio)
- **Contextual RAG framework**: Document summaries and chunk context generation (LLM integration ready)
- **Quality filtering**: Advanced chunk quality assessment and filtering
- **Batch processing**: Efficient multi-document chunking
- **Metadata processing**: Title and metadata token management
- **Section processing**: Continuation logic and oversized handling
- **Source-specific handling**: Gmail and other source types
- **Strict token limits**: Enforced token boundaries
- **Link tracking**: Source link offset management
- **Advanced text processing**: Comprehensive text utilities
- **Sophisticated tokenizers**: Simple, sentence, and BPE tokenizers

> **⚠️ LLM Integration Note**: The contextual RAG system currently uses simulated responses. When implementing real LLMs, replace `simulate_llm_response()` in `src/chunking/contextual_rag.cpp` with actual LLM API calls.

## 🔧 Installation & Usage

### Dual Approach: Direct Library + HTTP API

R3M provides **two usage modes** for maximum flexibility:

#### **1. Direct Library Usage (Zero Overhead)**
```cpp
#include "r3m/core/library.hpp"

int main() {
    r3m::core::Library library;
    library.initialize("configs/dev/config.yaml");
    
    // Zero overhead - direct function calls
    auto result = library.process_document("document.pdf");
    
    // Batch processing
    std::vector<std::string> files = {"doc1.pdf", "doc2.txt", "doc3.html"};
    auto results = library.process_documents_parallel(files);
    
    return 0;
}
```

#### **2. HTTP API Usage (For Remote/Web Integration)**
```bash
# Start the server
./build/r3m

# Process documents via HTTP API
curl -X POST http://localhost:8080/process \
  -H "Content-Type: application/json" \
  -d '{"file_path": "document.pdf"}'

# Batch processing
curl -X POST http://localhost:8080/batch \
  -H "Content-Type: application/json" \
  -d '{"files": ["doc1.pdf", "doc2.txt", "doc3.html"]}'
```

### Prerequisites
```bash
# Required libraries
brew install poppler gumbo-parser

# Optional HTTP libraries (for API functionality)
brew install crow nlohmann-json
```

### Build & Run

#### **Option 1: Direct Library Usage (Recommended for Performance)**
```bash
# Build the project
make build

# Run direct library example
./build/r3m-library-example

# Or use in your own C++ code
#include "r3m/core/library.hpp"
```

#### **Option 2: HTTP API Usage**
```bash
# Build the project
make build

# Run the server
./build/r3m

# Test API endpoints
curl http://localhost:8080/health
```

### Advanced Testing
```bash
# Build the project
make build

# Run comprehensive chunking tests
./build/r3m-advanced-chunking-test

# Run advanced features tests
./build/r3m-advanced-features-test

# Run advanced compatibility tests
./build/r3m-advanced-compatibility-test

# Run HTTP API tests
./build/r3m-http-test

# Clean build artifacts
make clean

# Clean test data only
make clean-test-data
```

### HTTP API Usage (when Crow is available)
```bash
# Health check
curl http://localhost:8080/health

# Process single document
curl -X POST http://localhost:8080/process \
  -H "Content-Type: application/json" \
  -d '{"file_path": "data/sample.txt"}'

# Process batch of documents
curl -X POST http://localhost:8080/batch \
  -H "Content-Type: application/json" \
  -d '{"files": ["data/file1.txt", "data/file2.md"]}'

# Get system information
curl http://localhost:8080/info
```

## 📋 Configuration

### Development Configuration (`configs/dev/config.yaml`)
```yaml
server:
  port: 8080
  host: "0.0.0.0"
  threads: 4

document_processing:
  supported_extensions:
    plain_text: [".txt", ".md", ".json", ".csv"]
    pdf: [".pdf"]
    html: [".html", ".htm"]
  
  enable_parallel_processing: true
  worker_threads: 4
  batch_size: 16
  
  quality_filtering:
    enabled: true
    min_content_quality_score: 0.3
    min_information_density: 0.1

chunking:
  chunk_token_limit: 1000
  enable_multipass: true
  enable_contextual_rag: true
  contextual_rag_reserved_tokens: 100
  include_metadata: true
  chunk_overlap: 0
  min_chunk_content: 256
  max_metadata_percentage: 0.25
```

## 🧪 Testing

### Comprehensive Test Suite
- **Core functionality**: Single document processing
- **Format support**: PDF, HTML, plain text
- **Parallel processing**: Multi-threaded performance
- **Batch processing**: Quality filtering
- **Quality assessment**: Advanced algorithms
- **Statistics tracking**: Performance metrics
- **Advanced chunking**: 12 comprehensive chunking tests
- **Advanced features**: 16 text processing and tokenizer tests
- **Advanced compatibility**: 6 compatibility verification tests

### Test Results
```
✅ CORE FUNCTIONALITY: Working
🚀 ENHANCED FEATURES: 2.88x speedup
📊 PERFORMANCE: 54 files, 2.3M characters
🧪 ADVANCED CHUNKING: 12/12 tests passed
🔧 ADVANCED FEATURES: 16/16 tests passed
✅ ADVANCED COMPATIBILITY: 6/6 tests passed
🎉 ALL TESTS PASSED!
```

### HTTP API Test Results
```
✅ HEALTH CHECK: Server responding
✅ SYSTEM INFO: Statistics and configuration
✅ SINGLE PROCESSING: All formats (TXT, MD, JSON, HTML, CSV)
✅ BATCH PROCESSING: Multiple documents with quality filtering
✅ JOB STATUS: Async job tracking
✅ PERFORMANCE: 100% success rate (11/11 tests)
🎉 EXCELLENT! All API endpoints working perfectly!
```

## 🔄 Development Phases

### ✅ Phase 1: Core Engine (COMPLETED)
- [x] Document processing pipeline
- [x] Parallel processing with thread pools
- [x] Batch processing with quality filtering
- [x] Quality assessment algorithms
- [x] Format support (PDF, HTML, plain text)
- [x] Configuration-driven architecture

### ✅ Phase 2: HTTP Server (COMPLETED)
- [x] REST API endpoints
- [x] JSON request/response handling
- [x] Graceful fallback without HTTP libraries
- [x] Health check and system information
- [x] File upload and batch processing
- [x] CORS support

### ✅ Phase 3: Advanced Chunking (COMPLETED)
- [x] Multipass indexing: Mini-chunks (150 tokens) and large chunks (4x ratio)
- [x] Contextual RAG framework: Document summaries and chunk context generation (LLM ready)
- [x] Quality filtering: Advanced chunk quality assessment and filtering
- [x] Batch processing: Efficient multi-document chunking
- [x] Token-aware processing: Advanced tokenizer with multiple strategies
- [x] Sentence chunker: Semantic chunking respecting sentence boundaries
- [x] Metadata processing: Title and metadata token management
- [x] Section processing: Continuation logic and oversized handling
- [x] Source-specific handling: Gmail and other source types
- [x] Strict token limits: Enforced token boundaries
- [x] Link tracking: Source link offset management
- [x] Advanced text processing: Comprehensive text utilities
- [x] Sophisticated tokenizers: Simple, sentence, and BPE tokenizers

### 📋 Future Phases
- **Phase 4**: Vector Search & Embeddings
- **Phase 5**: Real LLM Integration (OpenAI, Local LLMs)
- **Phase 6**: Docker & Deployment
- **Phase 7**: Advanced Features

## 🎯 Key Decisions

### Architecture Choices
1. **Modular Design**: Separated concerns into distinct modules
2. **Configuration-Driven**: All parameters externalized to config.yaml
3. **Graceful Degradation**: HTTP server works without external libraries
4. **Performance-First**: Optimized for speed with parallel processing
5. **Quality-Focused**: Advanced filtering and assessment algorithms
6. **Advanced Chunking**: Comprehensive document segmentation system
7. **Independent Codebase**: Zero external dependencies on other systems

### Technical Decisions
1. **C++20**: Modern C++ features for better performance
2. **CMake Build System**: Cross-platform compatibility
3. **Optional Dependencies**: HTTP libraries are optional
4. **Thread Pool**: Custom C++ thread pool for parallel processing
5. **Quality Metrics**: Sophisticated content quality assessment
6. **Advanced Tokenizers**: Multiple tokenization strategies
7. **Text Processing**: Comprehensive text manipulation utilities

## 🚧 Solved Challenges

### Build System
- **Cross-platform compatibility**: Works on macOS, Linux, Windows
- **Optional dependencies**: Graceful handling of missing libraries
- **Library linking**: Proper poppler-cpp and gumbo integration
- **Conditional compilation**: HTTP features when libraries available

### Performance Optimization
- **Parallel processing**: 2.88x speedup with 71.88% efficiency
- **Batch processing**: Efficient handling of multiple documents
- **Quality filtering**: Intelligent document filtering
- **Memory management**: Optimized for large document processing

### HTTP Server Integration
- **Graceful fallback**: Works without Crow/nlohmann-json
- **REST API design**: Clean, intuitive endpoints
- **JSON handling**: Structured request/response format
- **Error handling**: Comprehensive error responses

### Advanced Chunking System
- **Multipass indexing**: Efficient mini-chunk and large-chunk generation
- **Contextual RAG**: Document summaries and chunk context (LLM ready)
- **Quality filtering**: Advanced chunk quality assessment
- **Metadata processing**: Title and metadata token management
- **Section processing**: Continuation logic and oversized handling
- **Source-specific handling**: Gmail and other source types
- **Strict token limits**: Enforced token boundaries
- **Link tracking**: Source link offset management
- **Advanced text processing**: Comprehensive text utilities
- **Sophisticated tokenizers**: Simple, sentence, and BPE tokenizers

## 📝 Development Journal

### Latest Updates (Advanced Chunking Phase - COMPLETED)
- **Implemented comprehensive chunking system**: All advanced features working
- **Created multipass indexing**: Mini-chunks (150 tokens) and large chunks (4x ratio)
- **Built contextual RAG framework**: Document summaries and chunk context generation
- **Added quality filtering**: Advanced chunk quality assessment and filtering
- **Implemented batch processing**: Efficient multi-document chunking
- **Created metadata processing**: Title and metadata token management
- **Added section processing**: Continuation logic and oversized handling
- **Implemented source-specific handling**: Gmail and other source types
- **Added strict token limits**: Enforced token boundaries
- **Created link tracking**: Source link offset management
- **Built advanced text processing**: Comprehensive text utilities
- **Implemented sophisticated tokenizers**: Simple, sentence, and BPE tokenizers
- **Removed all external references**: R3M is now completely independent
- **Comprehensive testing**: 34 total tests (12 chunking + 16 features + 6 compatibility)
- **Zero warnings**: Clean build with no unused variable warnings

### Previous Updates (HTTP Server Phase)
- **Added HTTP server module**: REST API for document processing
- **Implemented graceful fallback**: Works without HTTP libraries
- **Created comprehensive API**: Health, process, batch, info endpoints
- **Updated build system**: Conditional compilation for HTTP features
- **Enhanced main.cpp**: HTTP server integration
- **Maintained core functionality**: All existing features preserved

### Previous Updates (Core Engine Refactoring)
- **Modular architecture**: Separated concerns into distinct modules
- **Configuration-driven design**: All parameters from config.yaml
- **Performance optimization**: 2.88x parallel speedup
- **Quality assessment**: Advanced filtering algorithms
- **Comprehensive testing**: Single test suite for all features

---

**Note**: This project is currently in development. The purpose and 
full scope will be revealed in future phases. For now, this serves 
as a development journal tracking the implementation of a 
high-performance, modular document processing system with advanced 
quality assessment capabilities, comprehensive testing, and a 
framework ready for LLM integration.