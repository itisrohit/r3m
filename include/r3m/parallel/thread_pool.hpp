#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <future>
#include <type_traits>

namespace r3m {
namespace core {
    struct DocumentResult; // Forward declaration
}

namespace parallel {

class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads);
    ~ThreadPool();

    // Disable copy constructor and assignment
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // Submit task to thread pool
    template<class F, class... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>;

    // Batch processing - simplified for easier template deduction
    std::vector<std::future<core::DocumentResult>> submit_batch(
        const std::vector<std::function<core::DocumentResult()>>& tasks);

    // Thread pool management
    void shutdown();
    bool is_shutdown() const;
    size_t get_thread_count() const { return threads_.size(); }
    size_t get_queue_size() const;

private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool shutdown_;
    
    void worker_thread();
};

// Template implementation
template<class F, class... Args>
auto ThreadPool::submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>> {
    using return_type = typename std::invoke_result_t<F, Args...>;
    
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    std::future<return_type> result = task->get_future();
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (shutdown_) {
            throw std::runtime_error("ThreadPool is shutdown");
        }
        tasks_.emplace([task]() { (*task)(); });
    }
    
    condition_.notify_one();
    return result;
}

} // namespace parallel
} // namespace r3m 