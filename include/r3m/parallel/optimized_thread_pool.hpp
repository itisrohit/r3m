#pragma once

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <vector>
#include <memory>
#include <type_traits>

#ifdef __linux__
#include <pthread.h>
#include <sched.h>
#endif

namespace r3m {
namespace parallel {

/**
 * @brief Optimized Thread Pool with Single Pool Strategy
 * 
 * Implements advanced parallel processing optimizations:
 * - Single pool strategy to avoid dual thread pool conflicts
 * - Thread affinity for better cache locality
 * - Work stealing for improved load balancing
 * - Memory pooling to reduce allocation overhead
 * - Optimal batch sizing based on CPU cores
 */
class OptimizedThreadPool {
public:
    explicit OptimizedThreadPool(size_t num_threads = 0);
    ~OptimizedThreadPool();
    
    // Disable copy constructor and assignment
    OptimizedThreadPool(const OptimizedThreadPool&) = delete;
    OptimizedThreadPool& operator=(const OptimizedThreadPool&) = delete;
    
    // Submit single task
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>;
    
    // Submit batch of tasks
    template<typename F>
    auto submit_batch(const std::vector<std::function<F()>>& tasks) -> std::vector<std::future<F>>;
    
    // Get current queue size
    size_t get_queue_size() const;
    
    // Check if shutdown
    bool is_shutdown() const;
    
    // Shutdown the thread pool
    void shutdown();
    
    // Get optimal batch size based on CPU cores
    static size_t get_optimal_batch_size();
    
    // Disable library parallelism to avoid conflicts
    static void disable_library_parallelism();

private:
    // Thread worker function
    void worker_thread(size_t thread_id);
    
    // Set thread affinity for better cache locality
    void set_thread_affinity(size_t thread_id);
    
    // Work stealing implementation
    std::function<void()> steal_task(size_t thread_id);
    
    // Memory pool for reducing allocation overhead
    class MemoryPool {
    public:
        MemoryPool(size_t pool_size = 1024 * 1024); // 1MB default
        ~MemoryPool();
        
        void* allocate(size_t size);
        void deallocate(void* ptr);
        
    private:
        struct Block {
            void* data;
            size_t size;
            bool used;
        };
        
        std::vector<Block> blocks_;
        std::mutex pool_mutex_;
    };
    
    // Thread-local storage for memory pools
    struct ThreadLocalData {
        std::unique_ptr<MemoryPool> memory_pool;
        std::queue<std::function<void()>> local_queue;
        std::mutex local_mutex;
        
        ThreadLocalData() : memory_pool(std::make_unique<MemoryPool>()) {}
    };
    
    // Member variables
    std::vector<std::thread> threads_;
    std::vector<std::unique_ptr<ThreadLocalData>> thread_data_;
    
    // Global task queue (for work stealing)
    std::queue<std::function<void()>> global_queue_;
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    
    // Control variables
    std::atomic<bool> shutdown_{false};
    std::atomic<size_t> active_tasks_{0};
    
    // Performance monitoring
    mutable std::mutex stats_mutex_;
    size_t total_tasks_processed_{0};
    std::atomic<size_t> work_steals_{0};
    double avg_task_time_ms_{0.0};
    
    // Optimal configuration
    static constexpr size_t MAX_QUEUE_SIZE = 10000;
    static constexpr size_t WORK_STEAL_THRESHOLD = 5;
    static constexpr size_t MEMORY_POOL_SIZE = 1024 * 1024; // 1MB per thread
};

// Template implementations
template<typename F, typename... Args>
auto OptimizedThreadPool::submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>> {
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
        
        global_queue_.emplace([task]() { (*task)(); });
        active_tasks_.fetch_add(1);
    }
    
    condition_.notify_one();
    return result;
}

template<typename F>
auto OptimizedThreadPool::submit_batch(const std::vector<std::function<F()>>& tasks) -> std::vector<std::future<F>> {
    std::vector<std::future<F>> futures;
    futures.reserve(tasks.size());
    
    for (const auto& task : tasks) {
        futures.push_back(submit(task));
    }
    
    return futures;
}

// Static methods
inline size_t OptimizedThreadPool::get_optimal_batch_size() {
    size_t cpu_cores = std::thread::hardware_concurrency();
    if (cpu_cores == 0) cpu_cores = 4; // Fallback
    
    // Optimal batch size: 2x number of CPU cores
    // This ensures good load balancing without overwhelming the system
    return cpu_cores * 2;
}

inline void OptimizedThreadPool::disable_library_parallelism() {
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