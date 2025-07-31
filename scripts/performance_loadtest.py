#!/usr/bin/env python3
"""
R3M Performance Load Testing Script

Basic Usage:
python scripts/performance_loadtest.py --url http://localhost:8080

For more options, check the bottom of the file.
"""

import argparse
import asyncio
import logging
import statistics
import time
from collections.abc import AsyncGenerator
from dataclasses import dataclass
from logging import getLogger
from typing import List, Dict, Any
import aiohttp
import json
import os

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
    handlers=[logging.StreamHandler()],
)

logger = getLogger(__name__)


@dataclass
class ProcessingMetrics:
    """Metrics for document processing operations"""
    operation_id: str
    total_time: float
    processing_time: float
    chunking_time: float
    total_chunks: int
    successful_chunks: int
    avg_quality_score: float
    avg_information_density: float
    memory_usage_mb: float
    throughput_per_second: float


class R3MPerformanceLoadTester:
    """Load tester for R3M document processing API"""
    
    def __init__(
        self,
        base_url: str,
        num_concurrent: int,
        documents_per_batch: int,
        test_duration_seconds: int,
    ):
        self.base_url = base_url
        self.num_concurrent = num_concurrent
        self.documents_per_batch = documents_per_batch
        self.test_duration_seconds = test_duration_seconds
        self.metrics: List[ProcessingMetrics] = []
        
    def create_test_document(self, size_kb: int) -> str:
        """Create a test document of specified size"""
        words = [
            "document", "processing", "system", "performance", "optimization",
            "chunking", "tokenization", "metadata", "analysis", "quality",
            "assessment", "parallel", "threading", "memory", "efficiency",
            "throughput", "latency", "benchmark", "profiling", "monitoring"
        ]
        
        import random
        document = ""
        target_size = size_kb * 1024
        
        while len(document) < target_size:
            # Generate a sentence
            sentence_length = random.randint(5, 25)
            sentence = " ".join(random.choices(words, k=sentence_length))
            sentence += ". "
            document += sentence
            
        return document[:target_size]
    
    async def create_test_files(self, session: aiohttp.ClientSession) -> List[str]:
        """Create test files for processing"""
        test_files = []
        sizes = [1, 5, 10, 25, 50]  # KB
        
        for i, size in enumerate(sizes):
            filename = f"data/loadtest_doc_{i}_{size}kb.txt"
            content = self.create_test_document(size)
            
            # Save file locally
            os.makedirs("data", exist_ok=True)
            with open(filename, "w") as f:
                f.write(content)
            
            test_files.append(filename)
            logger.info(f"Created test file: {filename} ({len(content)} bytes)")
        
        return test_files
    
    async def process_document(
        self, 
        session: aiohttp.ClientSession, 
        file_path: str
    ) -> ProcessingMetrics:
        """Process a single document and measure performance"""
        start_time = time.time()
        
        # Read file content
        with open(file_path, "rb") as f:
            file_content = f.read()
        
        # Prepare multipart form data
        data = aiohttp.FormData()
        data.add_field('file', file_content, filename=os.path.basename(file_path))
        
        # Send request to R3M API
        async with session.post(
            f"{self.base_url}/process",
            data=data
        ) as response:
            response.raise_for_status()
            result = await response.json()
            
            end_time = time.time()
            total_time = end_time - start_time
            
            # Extract metrics from response
            processing_time = result.get("processing_time", 0.0)
            chunking_time = result.get("chunking_time", 0.0)
            total_chunks = result.get("total_chunks", 0)
            successful_chunks = result.get("successful_chunks", 0)
            avg_quality_score = result.get("avg_quality_score", 0.0)
            avg_information_density = result.get("avg_information_density", 0.0)
            
            # Calculate throughput
            throughput_per_second = 1.0 / total_time if total_time > 0 else 0.0
            
            # Estimate memory usage (this would need to be implemented in R3M)
            memory_usage_mb = len(file_content) / (1024 * 1024)  # Rough estimate
            
            return ProcessingMetrics(
                operation_id=f"op_{int(start_time * 1000)}",
                total_time=total_time,
                processing_time=processing_time,
                chunking_time=chunking_time,
                total_chunks=total_chunks,
                successful_chunks=successful_chunks,
                avg_quality_score=avg_quality_score,
                avg_information_density=avg_information_density,
                memory_usage_mb=memory_usage_mb,
                throughput_per_second=throughput_per_second
            )
    
    async def process_batch(
        self, 
        session: aiohttp.ClientSession, 
        file_paths: List[str]
    ) -> List[ProcessingMetrics]:
        """Process a batch of documents"""
        tasks = []
        for file_path in file_paths:
            task = self.process_document(session, file_path)
            tasks.append(task)
        
        return await asyncio.gather(*tasks, return_exceptions=True)
    
    async def run_concurrent_session(self, session: aiohttp.ClientSession) -> None:
        """Run a single concurrent session"""
        test_files = await self.create_test_files(session)
        
        # Create multiple batches
        batches = []
        for i in range(self.documents_per_batch):
            batch_files = test_files * 2  # Repeat files to get more data
            batches.append(batch_files)
        
        # Process batches concurrently
        for batch_files in batches:
            try:
                batch_metrics = await self.process_batch(session, batch_files)
                
                # Filter out exceptions and add valid metrics
                for metric in batch_metrics:
                    if isinstance(metric, ProcessingMetrics):
                        self.metrics.append(metric)
                    else:
                        logger.error(f"Processing failed: {metric}")
                        
            except Exception as e:
                logger.error(f"Batch processing failed: {e}")
    
    async def run_load_test(self) -> None:
        """Run the complete load test"""
        logger.info(f"Starting R3M load test with {self.num_concurrent} concurrent sessions")
        logger.info(f"Test duration: {self.test_duration_seconds} seconds")
        
        start_time = time.time()
        
        async with aiohttp.ClientSession() as session:
            # Create concurrent sessions
            tasks = []
            for i in range(self.num_concurrent):
                task = self.run_concurrent_session(session)
                tasks.append(task)
            
            # Run all sessions concurrently
            await asyncio.gather(*tasks, return_exceptions=True)
        
        end_time = time.time()
        total_time = end_time - start_time
        
        self.print_results(total_time)
    
    def print_results(self, total_time: float) -> None:
        """Print comprehensive test results"""
        if not self.metrics:
            logger.error("No metrics collected during test")
            return
        
        print("\n" + "="*60)
        print("R3M PERFORMANCE LOAD TEST RESULTS")
        print("="*60)
        
        # Calculate statistics
        total_times = [m.total_time for m in self.metrics]
        processing_times = [m.processing_time for m in self.metrics]
        chunking_times = [m.chunking_time for m in self.metrics]
        total_chunks = [m.total_chunks for m in self.metrics]
        successful_chunks = [m.successful_chunks for m in self.metrics]
        throughputs = [m.throughput_per_second for m in self.metrics]
        
        print(f"\n📊 Test Summary:")
        print(f"   Total Operations: {len(self.metrics)}")
        print(f"   Test Duration: {total_time:.2f} seconds")
        print(f"   Concurrent Sessions: {self.num_concurrent}")
        print(f"   Documents per Batch: {self.documents_per_batch}")
        
        print(f"\n⏱️  Timing Statistics:")
        print(f"   Total Time - Avg: {statistics.mean(total_times):.3f}s, "
              f"Min: {min(total_times):.3f}s, Max: {max(total_times):.3f}s")
        print(f"   Processing Time - Avg: {statistics.mean(processing_times):.3f}s, "
              f"Min: {min(processing_times):.3f}s, Max: {max(processing_times):.3f}s")
        print(f"   Chunking Time - Avg: {statistics.mean(chunking_times):.3f}s, "
              f"Min: {min(chunking_times):.3f}s, Max: {max(chunking_times):.3f}s")
        
        print(f"\n📈 Throughput Statistics:")
        print(f"   Throughput - Avg: {statistics.mean(throughputs):.2f} ops/s, "
              f"Min: {min(throughputs):.2f} ops/s, Max: {max(throughputs):.2f} ops/s")
        
        print(f"\n🔢 Chunking Statistics:")
        print(f"   Total Chunks - Avg: {statistics.mean(total_chunks):.1f}, "
              f"Min: {min(total_chunks)}, Max: {max(total_chunks)}")
        print(f"   Successful Chunks - Avg: {statistics.mean(successful_chunks):.1f}, "
              f"Min: {min(successful_chunks)}, Max: {max(successful_chunks)}")
        
        # Calculate success rate
        success_rate = len([m for m in self.metrics if m.successful_chunks > 0]) / len(self.metrics) * 100
        print(f"\n✅ Success Rate: {success_rate:.1f}%")
        
        # Export results to CSV
        self.export_results_to_csv()
    
    def export_results_to_csv(self) -> None:
        """Export results to CSV file"""
        import csv
        
        filename = f"data/loadtest_results_{int(time.time())}.csv"
        os.makedirs("data", exist_ok=True)
        
        with open(filename, 'w', newline='') as csvfile:
            fieldnames = [
                'operation_id', 'total_time', 'processing_time', 'chunking_time',
                'total_chunks', 'successful_chunks', 'avg_quality_score',
                'avg_information_density', 'memory_usage_mb', 'throughput_per_second'
            ]
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            
            writer.writeheader()
            for metric in self.metrics:
                writer.writerow({
                    'operation_id': metric.operation_id,
                    'total_time': metric.total_time,
                    'processing_time': metric.processing_time,
                    'chunking_time': metric.chunking_time,
                    'total_chunks': metric.total_chunks,
                    'successful_chunks': metric.successful_chunks,
                    'avg_quality_score': metric.avg_quality_score,
                    'avg_information_density': metric.avg_information_density,
                    'memory_usage_mb': metric.memory_usage_mb,
                    'throughput_per_second': metric.throughput_per_second
                })
        
        print(f"\n📊 Results exported to: {filename}")


def main() -> None:
    """Main function to run the load test"""
    parser = argparse.ArgumentParser(description="R3M Performance Load Tester")
    parser.add_argument(
        "--url", 
        default="http://localhost:8080",
        help="R3M API base URL (default: http://localhost:8080)"
    )
    parser.add_argument(
        "--concurrent", 
        type=int, 
        default=5,
        help="Number of concurrent sessions (default: 5)"
    )
    parser.add_argument(
        "--batch-size", 
        type=int, 
        default=10,
        help="Number of documents per batch (default: 10)"
    )
    parser.add_argument(
        "--duration", 
        type=int, 
        default=60,
        help="Test duration in seconds (default: 60)"
    )
    
    args = parser.parse_args()
    
    # Create and run load tester
    load_tester = R3MPerformanceLoadTester(
        base_url=args.url,
        num_concurrent=args.concurrent,
        documents_per_batch=args.batch_size,
        test_duration_seconds=args.duration
    )
    
    try:
        asyncio.run(load_tester.run_load_test())
    except KeyboardInterrupt:
        logger.info("Load test interrupted by user")
    except Exception as e:
        logger.error(f"Load test failed: {e}")
        return 1
    
    return 0


if __name__ == "__main__":
    exit(main()) 