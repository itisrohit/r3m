#include "r3m/parallel/thread_pool.hpp"
#include "r3m/core/document_processor.hpp"

#include <stdexcept>

namespace r3m {
namespace parallel {

ThreadPool::ThreadPool(size_t num_threads) : shutdown_(false) {
    if (num_threads == 0) {
        num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) {
            // This will be set from config, but we need a reasonable fallback
            num_threads = 4; // Fallback - should be overridden by config
        }
    }
    
    // Start worker threads
    for (size_t i = 0; i < num_threads; ++i) {
        threads_.emplace_back(&ThreadPool::worker_thread, this);
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
}

void ThreadPool::shutdown() {
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

bool ThreadPool::is_shutdown() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return shutdown_;
}

size_t ThreadPool::get_queue_size() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return tasks_.size();
}

std::vector<std::future<core::DocumentResult>> ThreadPool::submit_batch(
    const std::vector<std::function<core::DocumentResult()>>& tasks) {
    
    std::vector<std::future<core::DocumentResult>> futures;
    futures.reserve(tasks.size());
    
    for (const auto& task : tasks) {
        futures.push_back(submit(task));
    }
    
    return futures;
}

void ThreadPool::worker_thread() {
    while (true) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait(lock, [this]() { 
                return !tasks_.empty() || shutdown_; 
            });
            
            if (shutdown_ && tasks_.empty()) {
                break;
            }
            
            if (!tasks_.empty()) {
                task = std::move(tasks_.front());
                tasks_.pop();
            }
        }
        
        if (task) {
            task();
        }
    }
}

} // namespace parallel
} // namespace r3m 