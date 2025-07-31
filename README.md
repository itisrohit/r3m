# R3M 

> **Development Journal**: A high-performance C++ document processing pipeline with HTTP API capabilities

## 🚀 Current Status: HTTP Server + Core Engine Complete

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

### 🎯 **Phase 3: Chunking Logic** - NEXT
- **Intelligent document chunking strategies**
- **Semantic boundary detection**
- **Configurable chunk sizes and overlap**
- **Chunk quality assessment**

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

## 🏗️ Architecture

### Core Modules
```
r3m/
├── core/           # Core processing engine
├── processing/     # Pipeline orchestration
├── quality/        # Quality assessment
├── parallel/       # Thread pool management
├── formats/        # Document format processors
├── server/         # HTTP server (modular)
└── api/            # API layer (modular)
    ├── routes/         # REST API endpoints
    ├── jobs/           # Async job tracking
    └── config/         # Configuration management
└── utils/          # Text utilities
```

### HTTP Server (Optional)
- **Modular Architecture**: `api::Routes`, `api::JobManager`, `api::Config` modules
- **Organized Structure**: Separate folders for routes, jobs, and config
- **Crow HTTP server**: Lightweight C++ web framework
- **JSON handling**: nlohmann/json library with fallback support
- **Graceful fallback**: Works without external HTTP libraries
- **Configuration-driven**: All settings from config.yaml
- **Job management**: Async processing with status tracking

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
```bash
# Build the project
make build

# Run the server (with or without HTTP libraries)
./build/r3m

# Run comprehensive tests
make test

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
```

## 🧪 Testing

### Comprehensive Test Suite
- **Core functionality**: Single document processing
- **Format support**: PDF, HTML, plain text
- **Parallel processing**: Multi-threaded performance
- **Batch processing**: Quality filtering
- **Quality assessment**: Advanced algorithms
- **Statistics tracking**: Performance metrics

### Test Results
```
✅ CORE FUNCTIONALITY: Working
🚀 ENHANCED FEATURES: 2.88x speedup
📊 PERFORMANCE: 54 files, 2.3M characters
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

### 🎯 Phase 3: Chunking Logic (NEXT)
- [ ] Intelligent document chunking
- [ ] Semantic boundary detection
- [ ] Configurable chunk sizes
- [ ] Chunk overlap strategies
- [ ] Chunk quality assessment
- [ ] Integration with existing pipeline

### 📋 Future Phases
- **Phase 4**: Vector Search & Embeddings
- **Phase 5**: Docker & Deployment
- **Phase 6**: Advanced Features

## 🎯 Key Decisions

### Architecture Choices
1. **Modular Design**: Separated concerns into distinct modules
2. **Configuration-Driven**: All parameters externalized to config.yaml
3. **Graceful Degradation**: HTTP server works without external libraries
4. **Performance-First**: Optimized for speed with parallel processing
5. **Quality-Focused**: Advanced filtering and assessment algorithms

### Technical Decisions
1. **C++20**: Modern C++ features for better performance
2. **CMake Build System**: Cross-platform compatibility
3. **Optional Dependencies**: HTTP libraries are optional
4. **Thread Pool**: Custom C++ thread pool for parallel processing
5. **Quality Metrics**: Sophisticated content quality assessment

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

## 📝 Development Journal

### Latest Updates (HTTP Server Phase)
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

**Note**: This project is currently in development. The purpose and full scope will be revealed in future phases. For now, this serves as a development journal tracking the implementation of a high-performance, modular document processing system with advanced quality assessment capabilities and comprehensive testing. 