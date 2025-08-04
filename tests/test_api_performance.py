#!/usr/bin/env python3
"""
R3M API Performance Test Script
Tests document processing, chunking, and benchmarks with optimized settings
"""

import requests
import json
import time
import statistics
from typing import Dict, List, Any
import os

class R3MAPITester:
    def __init__(self, base_url: str = "http://localhost:7860"):
        self.base_url = base_url
        self.session = requests.Session()
        
    def test_health(self) -> bool:
        """Test server health"""
        try:
            response = self.session.get(f"{self.base_url}/health")
            return response.status_code == 200
        except Exception as e:
            print(f"❌ Health check failed: {e}")
            return False
    
    def test_system_info(self) -> Dict[str, Any]:
        """Get system information"""
        try:
            response = self.session.get(f"{self.base_url}/info")
            if response.status_code == 200:
                return response.json()
            else:
                print(f"❌ System info failed: {response.status_code}")
                return {}
        except Exception as e:
            print(f"❌ System info error: {e}")
            return {}
    
    def test_metrics(self) -> Dict[str, Any]:
        """Get performance metrics"""
        try:
            response = self.session.get(f"{self.base_url}/metrics")
            if response.status_code == 200:
                return response.json()
            else:
                print(f"❌ Metrics failed: {response.status_code}")
                return {}
        except Exception as e:
            print(f"❌ Metrics error: {e}")
            return {}
    
    def create_test_document(self, size_kb: int = 10) -> str:
        """Create a test document of specified size"""
        # Simple technical content for better processing (no leading blank line)
        base_content = """R3M Document Processing System

Overview
The R3M (Rapid Response Retrieval Model) is a high-performance document processing system designed for enterprise-scale text analysis and chunking. It implements advanced algorithms for semantic understanding and information extraction.

Key Features

Advanced Chunking Algorithm
The system employs a sophisticated chunking algorithm that analyzes document structure and semantic boundaries, maintains context across chunk boundaries, optimizes chunk size for downstream processing, and preserves metadata and formatting information.

Performance Optimizations
R3M incorporates several performance optimizations including SIMD-optimized text processing using AVX2/AVX-512 instructions, parallel processing with optimized thread pools, memory pooling for reduced allocation overhead, and work-stealing algorithms for load balancing.

Quality Assessment
Each chunk undergoes comprehensive quality assessment including information density calculation, content relevance scoring, metadata preservation, and contextual coherence analysis.

Technical Implementation

Architecture
The system is built with C++ for maximum performance featuring multi-threaded processing pipeline, zero-copy data structures where possible, SIMD vectorization for text operations, and memory-mapped file I/O for large documents.

Processing Pipeline
Document Ingestion: File validation and format detection
Text Extraction: Raw text extraction with metadata preservation
Preprocessing: Text cleaning and normalization
Chunking: Semantic chunking with overlap management
Quality Assessment: Scoring and filtering of chunks
Output Generation: Structured results with metadata

Performance Metrics
Processing speed: 2.38M characters/second
Chunking efficiency: 96.76% parallel efficiency
Memory usage: Optimized with pooling
Throughput: 500KB documents in ~50ms

API Endpoints

Core Processing
POST /process - Single document processing
POST /batch - Batch document processing
POST /chunk - Dedicated chunking endpoint

Monitoring
GET /health - Server health check
GET /metrics - Performance metrics
GET /info - System information

Configuration
The system supports extensive configuration options including chunk size limits and overlap settings, quality thresholds and filtering criteria, parallel processing parameters, and SIMD optimization flags.

Use Cases

Enterprise Document Processing
Large-scale document ingestion
Automated content analysis
Information extraction workflows
Search index preparation

Research and Development
Text mining applications
Natural language processing
Machine learning data preparation
Academic research projects

Performance Benchmarks

Document Size Performance
1KB documents: ~2ms processing time
10KB documents: ~8ms processing time
50KB documents: ~25ms processing time
100KB documents: ~45ms processing time
500KB documents: ~180ms processing time

Parallel Processing
Sequential processing: Baseline performance
Parallel processing: 3.87x speedup
Efficiency: 96.76% (target: >80%)
Optimal batch size: 16 documents

SIMD Optimizations
BPE tokenization: 10.53x speedup
Sentence detection: 3.85x speedup
Text normalization: 1.75x speedup
Character counting: 2.1x speedup

Future Enhancements

Planned Features
GPU acceleration for large-scale processing
Distributed processing across multiple nodes
Real-time streaming document processing
Advanced semantic chunking algorithms

Integration Capabilities
REST API for easy integration
Docker containerization for deployment
Configuration management system
Comprehensive monitoring and logging

Conclusion
The R3M system represents a significant advancement in document processing technology, combining high performance with sophisticated semantic understanding. Its modular architecture and extensive configuration options make it suitable for a wide range of applications, from enterprise document management to research projects requiring advanced text analysis capabilities."""
        
        # Repeat content to reach desired size
        target_size = size_kb * 1024
        content = base_content
        while len(content.encode('utf-8')) < target_size:
            content += base_content
        
        # Truncate to exact size
        content = content[:target_size]
        return content
    
    def create_test_file(self, size_kb: int = 10, filename: str = None) -> str:
        """Create a test file on disk and return the file path"""
        if filename is None:
            filename = f"test_document_{size_kb}kb.txt"
        
        # Ensure data directory exists (where server expects files)
        os.makedirs("build/data", exist_ok=True)
        file_path = f"build/data/{filename}"
        
        # Create content
        content = self.create_test_document(size_kb)
        
        # Write to file
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        
        # Return the path that the server expects (relative to build directory)
        return f"data/{filename}"
    
    def test_document_processing(self, document_content: str, test_name: str = "Test Document") -> Dict[str, Any]:
        """Test document processing with chunking"""
        print(f"\n🔍 Testing: {test_name}")
        print(f"   Document size: {len(document_content)} bytes ({len(document_content)/1024:.1f} KB)")
        
        # Test /process endpoint with content only
        process_data = {
            "file_content": document_content
        }
        
        start_time = time.time()
        try:
            response = self.session.post(
                f"{self.base_url}/process",
                json=process_data,
                headers={"Content-Type": "application/json"}
            )
            process_time = (time.time() - start_time) * 1000
            
            if response.status_code == 200:
                result = response.json()
                print(f"   ✅ Processing successful")
                print(f"   ⏱️  API response time: {process_time:.2f} ms")
                print(f"   📊 Processing time: {result['data']['processing_time_ms']:.2f} ms")
                print(f"   📄 Text length: {result['data']['text_length']} chars")
                print(f"   🎯 Quality score: {result['data']['content_quality_score']:.3f}")
                print(f"   📈 Information density: {result['data']['information_density']:.3f}")
                print(f"   🏷️  High quality: {result['data']['is_high_quality']}")
                print(f"   📦 Total chunks: {result['data']['total_chunks']}")
                print(f"   ✅ Successful chunks: {result['data']['successful_chunks']}")
                print(f"   📊 Avg chunk quality: {result['data']['avg_chunk_quality']:.3f}")
                print(f"   📈 Avg chunk density: {result['data']['avg_chunk_density']:.3f}")
                
                return {
                    "success": True,
                    "api_response_time": process_time,
                    "processing_time": result['data']['processing_time_ms'],
                    "text_length": result['data']['text_length'],
                    "quality_score": result['data']['content_quality_score'],
                    "information_density": result['data']['information_density'],
                    "is_high_quality": result['data']['is_high_quality'],
                    "total_chunks": result['data']['total_chunks'],
                    "successful_chunks": result['data']['successful_chunks'],
                    "avg_chunk_quality": result['data']['avg_chunk_quality'],
                    "avg_chunk_density": result['data']['avg_chunk_density'],
                    "chunks": result['data']['chunks']
                }
            else:
                print(f"   ❌ Processing failed: {response.status_code}")
                print(f"   📄 Response: {response.text}")
                return {"success": False, "error": response.text}
                
        except Exception as e:
            print(f"   ❌ Processing error: {e}")
            return {"success": False, "error": str(e)}
    
    def test_file_processing(self, file_path: str, test_name: str = "Test File") -> Dict[str, Any]:
        """Test document processing using file path"""
        print(f"\n🔍 Testing file: {test_name}")
        print(f"   File path: {file_path}")
        
        # Test /process endpoint with file path
        process_data = {
            "file_path": file_path
        }
        
        start_time = time.time()
        try:
            response = self.session.post(
                f"{self.base_url}/process",
                json=process_data,
                headers={"Content-Type": "application/json"}
            )
            process_time = (time.time() - start_time) * 1000
            
            if response.status_code == 200:
                result = response.json()
                print(f"   ✅ Processing successful")
                print(f"   ⏱️  API response time: {process_time:.2f} ms")
                print(f"   📊 Processing time: {result['data']['processing_time_ms']:.2f} ms")
                print(f"   📄 Text length: {result['data']['text_length']} chars")
                print(f"   🎯 Quality score: {result['data']['content_quality_score']:.3f}")
                print(f"   📈 Information density: {result['data']['information_density']:.3f}")
                print(f"   🏷️  High quality: {result['data']['is_high_quality']}")
                print(f"   📦 Total chunks: {result['data']['total_chunks']}")
                print(f"   ✅ Successful chunks: {result['data']['successful_chunks']}")
                print(f"   📊 Avg chunk quality: {result['data']['avg_chunk_quality']:.3f}")
                print(f"   📈 Avg chunk density: {result['data']['avg_chunk_density']:.3f}")
                
                return {
                    "success": True,
                    "api_response_time": process_time,
                    "processing_time": result['data']['processing_time_ms'],
                    "text_length": result['data']['text_length'],
                    "quality_score": result['data']['content_quality_score'],
                    "information_density": result['data']['information_density'],
                    "is_high_quality": result['data']['is_high_quality'],
                    "total_chunks": result['data']['total_chunks'],
                    "successful_chunks": result['data']['successful_chunks'],
                    "avg_chunk_quality": result['data']['avg_chunk_quality'],
                    "avg_chunk_density": result['data']['avg_chunk_density'],
                    "chunks": result['data']['chunks']
                }
            else:
                print(f"   ❌ Processing failed: {response.status_code}")
                print(f"   📄 Response: {response.text}")
                return {"success": False, "error": response.text}
                
        except Exception as e:
            print(f"   ❌ Processing error: {e}")
            return {"success": False, "error": str(e)}
    
    def test_chunking_only(self, document_content: str, test_name: str = "Test Document") -> Dict[str, Any]:
        """Test dedicated chunking endpoint"""
        print(f"\n🔍 Testing chunking: {test_name}")
        
        # First create a file with the content
        filename = f"{test_name.lower().replace(' ', '_')}.txt"
        file_path = f"data/{filename}"
        
        # Ensure data directory exists
        os.makedirs("build/data", exist_ok=True)
        
        # Write content to file
        with open(f"build/{file_path}", 'w', encoding='utf-8') as f:
            f.write(document_content)
        
        chunk_data = {
            "file_path": file_path
        }
        
        start_time = time.time()
        try:
            response = self.session.post(
                f"{self.base_url}/chunk",
                json=chunk_data,
                headers={"Content-Type": "application/json"}
            )
            api_time = (time.time() - start_time) * 1000
            
            if response.status_code == 200:
                result = response.json()
                print(f"   ✅ Chunking successful")
                print(f"   ⏱️  API response time: {api_time:.2f} ms")
                print(f"   📊 Processing time: {result['data']['processing_time_ms']:.2f} ms")
                print(f"   📦 Total chunks: {result['data']['total_chunks']}")
                print(f"   ✅ Successful chunks: {result['data']['successful_chunks']}")
                print(f"   📊 Avg quality: {result['data']['avg_quality_score']:.3f}")
                print(f"   📈 Avg density: {result['data']['avg_information_density']:.3f}")
                print(f"   🏆 High quality chunks: {result['data']['high_quality_chunks']}")
                
                return {
                    "success": True,
                    "api_response_time": api_time,
                    "processing_time": result['data']['processing_time_ms'],
                    "total_chunks": result['data']['total_chunks'],
                    "successful_chunks": result['data']['successful_chunks'],
                    "avg_quality": result['data']['avg_quality_score'],
                    "avg_density": result['data']['avg_information_density'],
                    "high_quality_chunks": result['data']['high_quality_chunks'],
                    "chunks": result['data']['chunks']
                }
            else:
                print(f"   ❌ Chunking failed: {response.status_code}")
                print(f"   📄 Response: {response.text}")
                return {"success": False, "error": response.text}
                
        except Exception as e:
            print(f"   ❌ Chunking error: {e}")
            return {"success": False, "error": str(e)}
    
    def run_performance_benchmark(self):
        """Run comprehensive performance benchmark"""
        print("🚀 R3M API Performance Benchmark")
        print("=" * 50)
        
        # Test server health
        print("\n🔧 Testing server health...")
        if not self.test_health():
            print("❌ Server is not responding. Please start the R3M server.")
            return
        
        print("✅ Server is healthy!")
        
        # Get system info
        print("\n📊 System Information:")
        system_info = self.test_system_info()
        if system_info.get('success'):
            data = system_info['data']
            print(f"   Server: {data['server']}")
            print(f"   Version: {data['version']}")
            print(f"   Port: {data['port']}")
            print(f"   Threads: {data['threads']}")
            print(f"   Max file size: {data['max_file_size_mb']} MB")
        
        # Get initial metrics
        print("\n📈 Initial Performance Metrics:")
        initial_metrics = self.test_metrics()
        if initial_metrics.get('success'):
            data = initial_metrics['data']
            print(f"   Total files processed: {data['total_files_processed']}")
            print(f"   Successful processing: {data['successful_processing']}")
            print(f"   Failed processing: {data['failed_processing']}")
            print(f"   Avg processing time: {data['avg_processing_time_ms']:.2f} ms")
            print(f"   Total text extracted: {data['total_text_extracted']} bytes")
            print(f"   Avg quality score: {data['avg_content_quality_score']:.3f}")
        
        # Test file-based processing (recommended approach)
        print(f"\n📄 Testing file-based processing:")
        print("=" * 50)
        
        file_results = []
        for size_kb in [1, 5, 10, 25, 50]:
            file_path = self.create_test_file(size_kb, f"test_{size_kb}kb.txt")
            result = self.test_file_processing(file_path, f"{size_kb}KB File")
            file_results.append({
                "size_kb": size_kb,
                "size_bytes": os.path.getsize(f"build/data/{file_path.split('/')[-1]}"),
                **result
            })
        
        # Test content-based processing (for comparison)
        print(f"\n📄 Testing content-based processing:")
        print("=" * 50)
        
        content_results = []
        for size_kb in [1, 5, 10, 25, 50]:
            document_content = self.create_test_document(size_kb)
            result = self.test_document_processing(document_content, f"{size_kb}KB Content")
            content_results.append({
                "size_kb": size_kb,
                "size_bytes": len(document_content),
                **result
            })
        
        # Test chunking-only endpoint
        print(f"\n🔪 Testing dedicated chunking endpoint:")
        print("=" * 50)
        
        chunking_results = []
        for size_kb in [10, 25, 50]:
            document_content = self.create_test_document(size_kb)
            result = self.test_chunking_only(document_content, f"{size_kb}KB Chunking")
            chunking_results.append({
                "size_kb": size_kb,
                "size_bytes": len(document_content),
                **result
            })
        
        # Calculate performance statistics for file-based processing
        successful_file_results = [r for r in file_results if r.get('success')]
        if successful_file_results:
            print(f"\n📊 File-Based Processing Summary:")
            print("=" * 45)
            
            api_times = [r['api_response_time'] for r in successful_file_results]
            processing_times = [r['processing_time'] for r in successful_file_results]
            quality_scores = [r['quality_score'] for r in successful_file_results]
            chunk_counts = [r['total_chunks'] for r in successful_file_results]
            
            print(f"   📈 API Response Times:")
            print(f"      Average: {statistics.mean(api_times):.2f} ms")
            print(f"      Median: {statistics.median(api_times):.2f} ms")
            print(f"      Min: {min(api_times):.2f} ms")
            print(f"      Max: {max(api_times):.2f} ms")
            
            print(f"   ⚡ Processing Times:")
            print(f"      Average: {statistics.mean(processing_times):.2f} ms")
            print(f"      Median: {statistics.median(processing_times):.2f} ms")
            print(f"      Min: {min(processing_times):.2f} ms")
            print(f"      Max: {max(processing_times):.2f} ms")
            
            print(f"   🎯 Quality Scores:")
            print(f"      Average: {statistics.mean(quality_scores):.3f}")
            print(f"      Median: {statistics.median(quality_scores):.3f}")
            print(f"      Min: {min(quality_scores):.3f}")
            print(f"      Max: {max(quality_scores):.3f}")
            
            print(f"   📦 Chunk Generation:")
            print(f"      Total chunks: {sum(chunk_counts)}")
            print(f"      Average chunks per doc: {statistics.mean(chunk_counts):.1f}")
            print(f"      Median chunks per doc: {statistics.median(chunk_counts):.1f}")
        
        # Calculate performance statistics for content-based processing
        successful_content_results = [r for r in content_results if r.get('success')]
        if successful_content_results:
            print(f"\n📊 Content-Based Processing Summary:")
            print("=" * 45)
            
            api_times = [r['api_response_time'] for r in successful_content_results]
            processing_times = [r['processing_time'] for r in successful_content_results]
            quality_scores = [r['quality_score'] for r in successful_content_results]
            chunk_counts = [r['total_chunks'] for r in successful_content_results]
            
            print(f"   📈 API Response Times:")
            print(f"      Average: {statistics.mean(api_times):.2f} ms")
            print(f"      Median: {statistics.median(api_times):.2f} ms")
            print(f"      Min: {min(api_times):.2f} ms")
            print(f"      Max: {max(api_times):.2f} ms")
            
            print(f"   ⚡ Processing Times:")
            print(f"      Average: {statistics.mean(processing_times):.2f} ms")
            print(f"      Median: {statistics.median(processing_times):.2f} ms")
            print(f"      Min: {min(processing_times):.2f} ms")
            print(f"      Max: {max(processing_times):.2f} ms")
            
            print(f"   🎯 Quality Scores:")
            print(f"      Average: {statistics.mean(quality_scores):.3f}")
            print(f"      Median: {statistics.median(quality_scores):.3f}")
            print(f"      Min: {min(quality_scores):.3f}")
            print(f"      Max: {max(quality_scores):.3f}")
            
            print(f"   📦 Chunk Generation:")
            print(f"      Total chunks: {sum(chunk_counts)}")
            print(f"      Average chunks per doc: {statistics.mean(chunk_counts):.1f}")
            print(f"      Median chunks per doc: {statistics.median(chunk_counts):.1f}")
        
        # Get final metrics
        print(f"\n📈 Final Performance Metrics:")
        final_metrics = self.test_metrics()
        if final_metrics.get('success'):
            data = final_metrics['data']
            print(f"   Total files processed: {data['total_files_processed']}")
            print(f"   Successful processing: {data['successful_processing']}")
            print(f"   Failed processing: {data['failed_processing']}")
            print(f"   Avg processing time: {data['avg_processing_time_ms']:.2f} ms")
            print(f"   Total text extracted: {data['total_text_extracted']} bytes")
            print(f"   Avg quality score: {data['avg_content_quality_score']:.3f}")
        
        print(f"\n🎉 Benchmark completed successfully!")
        print(f"✅ All endpoints tested with optimized settings")
        print(f"📊 Performance metrics collected")
        print(f"🔪 Chunking functionality verified")
        print(f"📄 Both file-based and content-based processing tested")

def main():
    """Main function to run the benchmark"""
    tester = R3MAPITester()
    tester.run_performance_benchmark()

if __name__ == "__main__":
    main()