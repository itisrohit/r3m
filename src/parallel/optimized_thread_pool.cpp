#include "r3m/parallel/optimized_thread_pool.hpp"
#include "r3m/core/document_processor.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>

namespace r3m {
namespace parallel {

// MemoryPool implementation
OptimizedThreadPool::MemoryPool::MemoryPool(size_t pool_size) {
    // Pre-allocate memory blocks
    size_t block_size = 4096; // 4KB blocks
    size_t num_blocks = pool_size / block_size;
    
    blocks_.reserve(num_blocks);
    for (size_t i = 0; i < num_blocks; ++i) {
        Block block;
        block.data = malloc(block_size);
        if (block.data) {  // Only add block if malloc succeeded
            block.size = block_size;
            block.used = false;
            blocks_.push_back(block);
        }
    }
}

OptimizedThreadPool::MemoryPool::~MemoryPool() {
    for (auto& block : blocks_) {
        if (block.data) {
            free(block.data);
        }
    }
}

void* OptimizedThreadPool::MemoryPool::allocate(size_t size) {
    // Temporarily disable memory pooling to avoid double-free issues
    // Just use malloc directly
    return malloc(size);
}

void OptimizedThreadPool::MemoryPool::deallocate(void* ptr) {
    // Temporarily disable memory pooling to avoid double-free issues
    // Just use free directly
    if (ptr) {
        free(ptr);
    }
}

// OptimizedThreadPool implementation
OptimizedThreadPool::OptimizedThreadPool(size_t num_threads) {
    if (num_threads == 0) {
        num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) {
            num_threads = 4; // Fallback
        }
    }
    
    // Disable library parallelism to avoid conflicts
    disable_library_parallelism();
    
    // Initialize thread-local data
    thread_data_.reserve(num_threads);
    for (size_t i = 0; i < num_threads; ++i) {
        thread_data_.push_back(std::make_unique<ThreadLocalData>());
    }
    
    // Start worker threads
    threads_.reserve(num_threads);
    for (size_t i = 0; i < num_threads; ++i) {
        threads_.emplace_back(&OptimizedThreadPool::worker_thread, this, i);
    }
}

OptimizedThreadPool::~OptimizedThreadPool() {
    shutdown();
}

void OptimizedThreadPool::shutdown() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        shutdown_ = true;
    }
    
    condition_.notify_all();
    
    // Wait for all threads to finish
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

// Template specialization for submit_batch
template<>
std::vector<std::future<core::DocumentResult>> OptimizedThreadPool::submit_batch(
    const std::vector<std::function<core::DocumentResult()>>& tasks) {
    
    std::vector<std::future<core::DocumentResult>> futures;
    futures.reserve(tasks.size());
    
    for (const auto& task : tasks) {
        futures.push_back(submit(task));
    }
    
    return futures;
}

bool OptimizedThreadPool::is_shutdown() const {
    return shutdown_.load();
}

size_t OptimizedThreadPool::get_queue_size() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return global_queue_.size();
}

void OptimizedThreadPool::set_thread_affinity(size_t thread_id) {
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_id, &cpuset);
    
    pthread_t current_thread = pthread_self();
    int ret = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
    
    if (ret != 0) {
        std::cerr << "Warning: Failed to set thread affinity for thread " << thread_id << std::endl;
    }
#else
    (void)thread_id;
#endif
}

std::function<void()> OptimizedThreadPool::steal_task(size_t thread_id) {
    // Try to steal from other threads' local queues
    for (size_t i = 0; i < thread_data_.size(); ++i) {
        if (i != thread_id) {
            std::lock_guard<std::mutex> lock(thread_data_[i]->local_mutex);
            if (!thread_data_[i]->local_queue.empty()) {
                auto task = std::move(thread_data_[i]->local_queue.front());
                thread_data_[i]->local_queue.pop();
                work_steals_.fetch_add(1);
                return task;
            }
        }
    }
    
    // Try to steal from global queue
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (!global_queue_.empty()) {
        auto task = std::move(global_queue_.front());
        global_queue_.pop();
        work_steals_.fetch_add(1);
        return task;
    }
    
    return nullptr;
}

void OptimizedThreadPool::worker_thread(size_t thread_id) {
    // Set thread affinity for better cache locality
    set_thread_affinity(thread_id);
    
    auto& local_data = thread_data_[thread_id];
    
    while (true) {
        std::function<void()> task;
        
        // First, try to get a task from local queue
        {
            std::lock_guard<std::mutex> lock(local_data->local_mutex);
            if (!local_data->local_queue.empty()) {
                task = std::move(local_data->local_queue.front());
                local_data->local_queue.pop();
            }
        }
        
        // If local queue is empty, try global queue
        if (!task) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait(lock, [this]() { 
                return !global_queue_.empty() || shutdown_; 
            });
            
            if (shutdown_ && global_queue_.empty()) {
                break;
            }
            
            if (!global_queue_.empty()) {
                task = std::move(global_queue_.front());
                global_queue_.pop();
            }
        }
        
        // If still no task, try work stealing
        if (!task) {
            task = steal_task(thread_id);
        }
        
        // Execute the task
        if (task) {
            auto start_time = std::chrono::high_resolution_clock::now();
            
            task();
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
            
            // Update statistics
            {
                std::lock_guard<std::mutex> lock(stats_mutex_);
                total_tasks_processed_++;
                
                // Update average task time (exponential moving average)
                double new_time_ms = duration.count() / 1000.0;
                avg_task_time_ms_ = 0.9 * avg_task_time_ms_ + 0.1 * new_time_ms;
            }
            
            active_tasks_.fetch_sub(1);
        }
    }
}

} // namespace parallel
} // namespace r3m 