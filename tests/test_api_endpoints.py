#!/usr/bin/env python3
"""
R3M HTTP API Test Suite
Tests all endpoints with document processing and enhancements
"""

import requests
import json
import os
import time
from pathlib import Path

API_URL = "http://localhost:8080"

def print_section(title):
    print("\n" + "="*60)
    print(f"🔍 {title}")
    print("="*60)

def print_subsection(title):
    print(f"\n📋 {title}")
    print("-" * 40)

def test_health():
    """Test /health endpoint"""
    print_section("Health Check")
    try:
        r = requests.get(f"{API_URL}/health", timeout=5)
        print(f"Status: {r.status_code}")
        print(f"Response: {r.text}")
        
        if r.status_code == 200:
            data = r.json()
            assert data.get("success") == True
            print("✅ Health check passed")
            return True
        else:
            print("❌ Health check failed")
            return False
    except Exception as e:
        print(f"❌ Health check error: {e}")
        return False

def test_info():
    """Test /info endpoint"""
    print_section("System Information")
    try:
        r = requests.get(f"{API_URL}/info", timeout=5)
        print(f"Status: {r.status_code}")
        print(f"Response: {r.text}")
        
        if r.status_code == 200:
            data = r.json()
            assert data.get("success") == True
            print("✅ System info retrieved")
            return True
        else:
            print("❌ System info failed")
            return False
    except Exception as e:
        print(f"❌ System info error: {e}")
        return False

def create_test_files():
    """Create test files for different formats"""
    print_section("Creating Test Files")
    
    test_files = []
    
    # Create data directory if it doesn't exist
    data_dir = Path("data")
    data_dir.mkdir(exist_ok=True)
    
    # Test files with different content types
    files_content = {
        "data/test_plain.txt": "This is a plain text document with some technical content. It contains information about C++ programming, document processing, and API development. The content includes various technical terms like HTTP, JSON, REST, and other programming concepts.",
        
        "data/test_markdown.md": """# Technical Document

This is a markdown document with **bold text** and *italic text*.

## Features
- Document processing
- Parallel execution
- Quality assessment
- API endpoints

## Code Example
```cpp
#include <iostream>
int main() {
    std::cout << "Hello World!" << std::endl;
    return 0;
}
```""",
        
        "data/test_json.json": """{
    "title": "Technical Specification",
    "version": "1.0.0",
    "features": [
        "Document Processing",
        "Parallel Execution", 
        "Quality Assessment",
        "HTTP API"
    ],
    "config": {
        "port": 8080,
        "threads": 4,
        "batch_size": 16
    }
}""",
        
        "data/test_html.html": """<!DOCTYPE html>
<html>
<head>
    <title>Technical Documentation</title>
</head>
<body>
    <h1>R3M Document Processing</h1>
    <p>This is an HTML document with technical content about:</p>
    <ul>
        <li>C++ Programming</li>
        <li>HTTP Server Implementation</li>
        <li>Document Processing Pipeline</li>
        <li>Quality Assessment Algorithms</li>
    </ul>
    <script>console.log("This should be filtered out");</script>
</body>
</html>""",
        
        "data/test_csv.csv": """Name,Type,Description
DocumentProcessor,Class,Main orchestrator for document processing
PipelineOrchestrator,Class,Coordinates processing stages
QualityAssessor,Class,Evaluates content quality
ThreadPool,Class,Manages parallel execution
FormatProcessor,Class,Handles different file formats""",
        
        "data/test_technical.txt": """Advanced Technical Document

This document contains sophisticated technical content with:
- Complex algorithms for document processing
- Parallel execution strategies using std::thread
- Quality assessment metrics (0.0-1.0 scale)
- Information density calculations
- Batch processing with configurable sizes
- HTTP API endpoints for RESTful access

Technical specifications:
- C++20 standard compliance
- CMake build system integration
- Cross-platform compatibility
- Optional dependency management
- Graceful fallback mechanisms

Performance metrics:
- Parallel speedup: 2.88x improvement
- Processing throughput: 54 files/second
- Quality filtering: Advanced content scoring
- Memory optimization: Efficient data structures"""
    }
    
    for file_path, content in files_content.items():
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        test_files.append(file_path)
        print(f"✅ Created: {file_path}")
    
    return test_files

def test_process_single(file_path):
    """Test /process endpoint with single document"""
    print_subsection(f"Single Document: {os.path.basename(file_path)}")
    
    try:
        data = {"file_path": file_path}
        r = requests.post(f"{API_URL}/process", json=data, timeout=10)
        
        print(f"Status: {r.status_code}")
        if r.status_code == 200:
            response_data = r.json()
            print(f"Success: {response_data.get('success')}")
            
            if response_data.get('success'):
                result = response_data.get('data', {})
                print(f"Processing Success: {result.get('processing_success')}")
                print(f"Content Length: {len(result.get('content', ''))}")
                print(f"Quality Score: {result.get('quality_score', 'N/A')}")
                print(f"Information Density: {result.get('information_density', 'N/A')}")
                print("✅ Single document processing passed")
                return True
            else:
                print(f"❌ Processing failed: {response_data.get('error', 'Unknown error')}")
                return False
        else:
            print(f"❌ HTTP error: {r.status_code}")
            return False
            
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def test_process_batch(file_paths):
    """Test /batch endpoint with multiple documents"""
    print_section("Batch Document Processing")
    
    try:
        data = {"files": file_paths}
        r = requests.post(f"{API_URL}/batch", json=data, timeout=30)
        
        print(f"Status: {r.status_code}")
        if r.status_code == 200:
            response_data = r.json()
            print(f"Success: {response_data.get('success')}")
            
            if response_data.get('success'):
                result = response_data.get('data', {})
                results = result.get('results', [])
                
                print(f"Total files: {len(file_paths)}")
                print(f"Processed results: {len(results)}")
                
                # Show results for each file
                for i, file_result in enumerate(results):
                    if file_result.get('processing_success', False):
                        print(f"  ✅ {os.path.basename(file_paths[i])}: "
                              f"Quality={file_result.get('content_quality_score', 'N/A')}, "
                              f"Density={file_result.get('information_density', 'N/A')}")
                    else:
                        error_msg = file_result.get('error_message', 'Unknown error')
                        print(f"  ❌ {os.path.basename(file_paths[i])}: {error_msg}")
                
                # Show batch statistics
                stats = result.get('statistics', {})
                if stats:
                    print(f"\n📊 Batch Statistics:")
                    print(f"  Total processing time: {stats.get('total_time_ms', 'N/A')}ms")
                    print(f"  Average time per file: {stats.get('avg_time_per_file_ms', 'N/A')}ms")
                    print(f"  Files processed: {stats.get('files_processed', 'N/A')}")
                    print(f"  Files filtered: {stats.get('files_filtered', 'N/A')}")
                
                print("✅ Batch processing passed")
                return True
            else:
                print(f"❌ Batch processing failed: {response_data.get('error', 'Unknown error')}")
                return False
        else:
            print(f"❌ HTTP error: {r.status_code}")
            return False
            
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def test_job_status():
    """Test /job/{id} endpoint (if async jobs are supported)"""
    print_section("Job Status Check")
    
    try:
        # Try to get status for a non-existent job
        job_id = "test-job-123"
        r = requests.get(f"{API_URL}/job/{job_id}", timeout=5)
        
        print(f"Status: {r.status_code}")
        print(f"Response: {r.text}")
        
        # This might return 404 or a proper response
        if r.status_code in [200, 404]:
            print("✅ Job status endpoint responded")
            return True
        else:
            print("❌ Job status endpoint failed")
            return False
            
    except Exception as e:
        print(f"❌ Job status error: {e}")
        return False

def run_performance_test():
    """Run a performance test with multiple files"""
    print_section("Performance Test")
    
    # Create multiple test files for performance testing
    performance_files = []
    for i in range(10):
        file_path = f"data/performance_test_{i}.txt"
        content = f"Performance test document {i} with technical content about C++ programming, HTTP APIs, and document processing. This document contains various technical terms and concepts related to software development and system architecture. Document {i} includes information about parallel processing, thread pools, and quality assessment algorithms."
        
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        performance_files.append(file_path)
    
    print(f"Created {len(performance_files)} performance test files")
    
    # Test batch processing with performance files
    start_time = time.time()
    success = test_process_batch(performance_files)
    end_time = time.time()
    
    if success:
        print(f"⏱️  Performance test completed in {end_time - start_time:.2f} seconds")
    
    # Clean up performance test files
    for file_path in performance_files:
        if os.path.exists(file_path):
            os.remove(file_path)
    
    return success

def main():
    """Main test function"""
    print("🚀 R3M HTTP API Test Suite")
    print("Testing all endpoints with document processing and enhancements")
    
    results = {
        "health": False,
        "info": False,
        "single_processing": [],
        "batch_processing": False,
        "job_status": False,
        "performance": False
    }
    
    # 1. Health check
    results["health"] = test_health()
    
    # 2. System info
    results["info"] = test_info()
    
    # 3. Create test files
    test_files = create_test_files()
    
    # 4. Single document processing (all formats)
    print_section("Single Document Processing Tests")
    for file_path in test_files:
        success = test_process_single(file_path)
        results["single_processing"].append({
            "file": os.path.basename(file_path),
            "success": success
        })
    
    # 5. Batch processing
    results["batch_processing"] = test_process_batch(test_files)
    
    # 6. Job status (if supported)
    results["job_status"] = test_job_status()
    
    # 7. Performance test
    results["performance"] = run_performance_test()
    
    # 8. Summary
    print_section("Test Results Summary")
    
    print(f"Health Check: {'✅ PASS' if results['health'] else '❌ FAIL'}")
    print(f"System Info: {'✅ PASS' if results['info'] else '❌ FAIL'}")
    print(f"Job Status: {'✅ PASS' if results['job_status'] else '❌ FAIL'}")
    print(f"Batch Processing: {'✅ PASS' if results['batch_processing'] else '❌ FAIL'}")
    print(f"Performance Test: {'✅ PASS' if results['performance'] else '❌ FAIL'}")
    
    print("\nSingle Document Processing:")
    for result in results["single_processing"]:
        status = "✅ PASS" if result["success"] else "❌ FAIL"
        print(f"  {result['file']}: {status}")
    
    # Calculate overall success rate
    total_tests = 1 + 1 + 1 + 1 + 1 + len(results["single_processing"])
    passed_tests = sum([
        results["health"],
        results["info"], 
        results["job_status"],
        results["batch_processing"],
        results["performance"]
    ]) + sum(1 for r in results["single_processing"] if r["success"])
    
    success_rate = (passed_tests / total_tests) * 100
    
    print(f"\n📊 Overall Success Rate: {success_rate:.1f}% ({passed_tests}/{total_tests})")
    
    if success_rate >= 90:
        print("🎉 Excellent! All major tests passed!")
    elif success_rate >= 70:
        print("✅ Good! Most tests passed!")
    else:
        print("⚠️  Some tests failed. Check the server logs.")
    
    # Clean up test files
    print_section("Cleanup")
    for file_path in test_files:
        if os.path.exists(file_path):
            os.remove(file_path)
            print(f"🗑️  Removed: {file_path}")

if __name__ == "__main__":
    main() 