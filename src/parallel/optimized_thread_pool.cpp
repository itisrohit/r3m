#include "r3m/parallel/optimized_thread_pool.hpp"
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
        block.size = block_size;
        block.used = false;
        blocks_.push_back(block);
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
    std::lock_guard<std::mutex> lock(pool_mutex_);
    
    // Find an available block
    for (auto& block : blocks_) {
        if (!block.used && block.size >= size) {
            block.used = true;
            return block.data;
        }
    }
    
    // If no suitable block found, fallback to malloc
    return malloc(size);
}

void OptimizedThreadPool::MemoryPool::deallocate(void* ptr) {
    std::lock_guard<std::mutex> lock(pool_mutex_);
    
    // Find the block and mark it as available
    for (auto& block : blocks_) {
        if (block.data == ptr) {
            block.used = false;
            return;
        }
    }
    
    // If not found in pool, fallback to free
    free(ptr);
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