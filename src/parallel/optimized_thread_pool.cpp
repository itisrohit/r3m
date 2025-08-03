#include "r3m/parallel/optimized_thread_pool.hpp"
#include "r3m/core/document_processor.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>

namespace r3m {
namespace parallel {

// Thread-safe MemoryPool implementation
OptimizedThreadPool::MemoryPool::MemoryPool(size_t pool_size) 
    : total_allocated_(0), pool_size_(0) {  // Always disable memory pooling for now
    (void)pool_size; // Suppress unused parameter warning
    
    // Check environment variable to completely disable memory pooling
    const char* disable_pooling = std::getenv("R3M_ENABLE_MEMORY_POOLING");
    if (disable_pooling && std::string(disable_pooling) == "false") {
        pool_size_ = 0;
    }
}

OptimizedThreadPool::MemoryPool::~MemoryPool() {
    // No cleanup needed since we're not using memory pooling
}

void* OptimizedThreadPool::MemoryPool::allocate(size_t size) {
    // Always use standard allocation to avoid memory corruption
    return malloc(size);
}

void OptimizedThreadPool::MemoryPool::deallocate(void* ptr) {
    if (!ptr) return;
    
    // Always use standard deallocation
    free(ptr);
}

void OptimizedThreadPool::MemoryPool::add_block(size_t size) {
    (void)size; // Suppress unused parameter warning
    // No-op since we're not using memory pooling
}

// OptimizedThreadPool implementation
OptimizedThreadPool::OptimizedThreadPool(size_t num_threads) {
    if (num_threads == 0) {
        // Check environment variable for thread count
        const char* max_workers = std::getenv("R3M_MAX_WORKERS");
        if (max_workers) {
            num_threads = std::stoul(max_workers);
        } else {
            num_threads = std::thread::hardware_concurrency();
            if (num_threads == 0) {
                num_threads = 4; // Fallback
            }
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
        auto packaged_task = std::make_shared<std::packaged_task<core::DocumentResult()>>(task);
        auto future = packaged_task->get_future();
        
        // Submit the packaged task
        submit([packaged_task]() {
            (*packaged_task)();
        });
        
        futures.push_back(std::move(future));
    }
    
    return futures;
}

std::function<void()> OptimizedThreadPool::steal_task(size_t thread_id) {
    // Try to steal from other threads' local queues
    for (size_t i = 0; i < thread_data_.size(); ++i) {
        if (i == thread_id) continue;
        
        auto& other_data = thread_data_[i];
        std::lock_guard<std::mutex> lock(other_data->local_mutex);
        
        if (!other_data->local_queue.empty()) {
            auto task = std::move(other_data->local_queue.front());
            other_data->local_queue.pop();
            work_steals_.fetch_add(1);
            return task;
        }
    }
    
    // Try global queue as last resort
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (!global_queue_.empty()) {
        auto task = std::move(global_queue_.front());
        global_queue_.pop();
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
        
        // First, try to get a task from local queue (most common case)
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
            condition_.wait(lock, [this] { return !global_queue_.empty() || shutdown_; });
            
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
        
        // Execute the task if we have one
        if (task) {
            auto start_time = std::chrono::steady_clock::now();
            
            try {
                task();
            } catch (const std::exception& e) {
                std::cerr << "Task execution failed: " << e.what() << std::endl;
            }
            
            // Update statistics
            {
                std::lock_guard<std::mutex> lock(stats_mutex_);
                total_tasks_processed_++;
                
                auto end_time = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
                double new_time_ms = duration.count() / 1000.0;
                avg_task_time_ms_ = 0.9 * avg_task_time_ms_ + 0.1 * new_time_ms;
            }
            
            active_tasks_.fetch_sub(1);
        }
    }
}

void OptimizedThreadPool::set_thread_affinity(size_t thread_id) {
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_id % CPU_SETSIZE, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
#else
    (void)thread_id; // Suppress unused parameter warning on non-Linux systems
#endif
}

size_t OptimizedThreadPool::get_queue_size() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return global_queue_.size();
}

bool OptimizedThreadPool::is_shutdown() const {
    return shutdown_.load();
}

// Static methods
size_t OptimizedThreadPool::get_optimal_batch_size() {
    // Check environment variable for batch size
    const char* batch_size = std::getenv("R3M_OPTIMAL_BATCH_SIZE");
    if (batch_size) {
        return std::stoul(batch_size);
    }
    
    size_t cpu_cores = std::thread::hardware_concurrency();
    if (cpu_cores == 0) cpu_cores = 4; // Fallback
    
    // Optimal batch size: 2x number of CPU cores
    // This ensures good load balancing without overwhelming the system
    return cpu_cores * 2;
}

void OptimizedThreadPool::disable_library_parallelism() {
    // Disable OpenBLAS threading
    setenv("OPENBLAS_NUM_THREADS", "1", 1);
    
    // Disable Intel MKL threading
    setenv("MKL_NUM_THREADS", "1", 1);
    
    // Disable OpenMP threading
    setenv("OMP_NUM_THREADS", "1", 1);
    
    // Disable BLIS threading
    setenv("BLIS_NUM_THREADS", "1", 1);
    
    // Disable NumPy threading
    setenv("NUMEXPR_NUM_THREADS", "1", 1);
}

} // namespace parallel
} // namespace r3m 