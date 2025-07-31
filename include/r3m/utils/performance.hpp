#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <numeric>
#include <algorithm>

namespace r3m {
namespace utils {

/**
 * @brief Performance timing utilities for R3M
 * 
 * Provides comprehensive performance measurement and benchmarking
 * capabilities similar to industry standard timing systems.
 */
class PerformanceUtils {
public:
    /**
     * @brief Performance metrics structure
     */
    struct PerformanceMetrics {
        std::string function_name;
        double execution_time_ms;
        size_t memory_usage_bytes;
        size_t input_size;
        size_t output_size;
        bool success;
        std::string error_message;
        
        PerformanceMetrics() : execution_time_ms(0.0), memory_usage_bytes(0), 
                             input_size(0), output_size(0), success(false) {}
    };
    
    /**
     * @brief Benchmark results structure
     */
    struct BenchmarkResults {
        std::string benchmark_name;
        size_t iterations;
        double total_time_ms;
        double avg_time_ms;
        double min_time_ms;
        double max_time_ms;
        double std_deviation_ms;
        double throughput_per_second;
        size_t total_memory_bytes;
        double avg_memory_bytes;
        
        BenchmarkResults() : iterations(0), total_time_ms(0.0), avg_time_ms(0.0),
                           min_time_ms(0.0), max_time_ms(0.0), std_deviation_ms(0.0),
                           throughput_per_second(0.0), total_memory_bytes(0), avg_memory_bytes(0.0) {}
    };
    
    /**
     * @brief Performance profiler class
     */
    class Profiler {
    public:
        Profiler(const std::string& name);
        ~Profiler();
        
        void start();
        void stop();
        PerformanceMetrics get_metrics() const;
        
    private:
        std::string name_;
        std::chrono::high_resolution_clock::time_point start_time_;
        std::chrono::high_resolution_clock::time_point end_time_;
        bool is_running_;
        PerformanceMetrics metrics_;
    };
    
    /**
     * @brief Benchmark runner class
     */
    class BenchmarkRunner {
    public:
        BenchmarkRunner(const std::string& name, size_t iterations = 100);
        
        template<typename Func, typename... Args>
        BenchmarkResults run_benchmark(Func&& func, Args&&... args) {
            BenchmarkResults results;
            results.benchmark_name = name_;
            results.iterations = iterations_;
            
            std::vector<double> execution_times;
            std::vector<size_t> memory_usage;
            
            // Warmup iterations
            for (size_t i = 0; i < warmup_iterations_; ++i) {
                func(std::forward<Args>(args)...);
            }
            
            // Actual benchmark iterations
            for (size_t i = 0; i < iterations_; ++i) {
                size_t memory_before = memory_tracking_ ? get_current_memory_usage() : 0;
                
                auto start_time = std::chrono::high_resolution_clock::now();
                func(std::forward<Args>(args)...);
                auto end_time = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration<double, std::milli>(end_time - start_time);
                execution_times.push_back(duration.count());
                
                if (memory_tracking_) {
                    size_t memory_after = get_current_memory_usage();
                    memory_usage.push_back(memory_after - memory_before);
                }
            }
            
            // Calculate statistics
            results.total_time_ms = std::accumulate(execution_times.begin(), execution_times.end(), 0.0);
            results.avg_time_ms = results.total_time_ms / iterations_;
            results.min_time_ms = *std::min_element(execution_times.begin(), execution_times.end());
            results.max_time_ms = *std::max_element(execution_times.begin(), execution_times.end());
            results.std_deviation_ms = calculate_std_deviation(execution_times);
            results.throughput_per_second = 1000.0 / results.avg_time_ms;
            
            if (memory_tracking_ && !memory_usage.empty()) {
                results.total_memory_bytes = std::accumulate(memory_usage.begin(), memory_usage.end(), 0ULL);
                results.avg_memory_bytes = static_cast<double>(results.total_memory_bytes) / memory_usage.size();
            }
            
            return results;
        }
        
        void set_warmup_iterations(size_t count);
        void set_memory_tracking(bool enabled);
        
    private:
        std::string name_;
        size_t iterations_;
        size_t warmup_iterations_;
        bool memory_tracking_;
        
        double calculate_std_deviation(const std::vector<double>& times);
        size_t get_current_memory_usage();
    };
    
    /**
     * @brief Performance monitor for continuous monitoring
     */
    class PerformanceMonitor {
    public:
        PerformanceMonitor();
        
        void start_monitoring();
        void stop_monitoring();
        void add_metric(const std::string& name, double value);
        void add_metric(const std::string& name, size_t value);
        
        std::unordered_map<std::string, std::vector<double>> get_metrics() const;
        void print_summary() const;
        void export_to_csv(const std::string& filename) const;
        
    private:
        std::unordered_map<std::string, std::vector<double>> metrics_;
        std::chrono::high_resolution_clock::time_point start_time_;
        bool is_monitoring_;
    };
    
    // Static utility functions
    static double get_current_time_ms();
    static size_t get_current_memory_usage();
    static std::string format_time(double milliseconds);
    static std::string format_memory(size_t bytes);
    static std::string format_throughput(double operations_per_second);
    
    // Performance decorator (similar to industry standard timing decorators)
    template<typename Func>
    static auto time_function(const std::string& name, Func&& func) -> decltype(func()) {
        Profiler profiler(name);
        profiler.start();
        auto result = func();
        profiler.stop();
        
        auto metrics = profiler.get_metrics();
        std::cout << "⏱️  " << name << " took " << format_time(metrics.execution_time_ms) << std::endl;
        
        return result;
    }
    
    // Memory tracking decorator
    template<typename Func>
    static auto track_memory(const std::string& name, Func&& func) -> decltype(func()) {
        size_t memory_before = get_current_memory_usage();
        auto result = func();
        size_t memory_after = get_current_memory_usage();
        
        std::cout << "💾 " << name << " memory: " << format_memory(memory_after - memory_before) << std::endl;
        
        return result;
    }
};

// Inline implementations for header-only usage
inline PerformanceUtils::Profiler::Profiler(const std::string& name) 
    : name_(name), is_running_(false) {}

inline PerformanceUtils::Profiler::~Profiler() {
    if (is_running_) {
        stop();
    }
}

inline void PerformanceUtils::Profiler::start() {
    start_time_ = std::chrono::high_resolution_clock::now();
    is_running_ = true;
}

inline void PerformanceUtils::Profiler::stop() {
    if (is_running_) {
        end_time_ = std::chrono::high_resolution_clock::now();
        is_running_ = false;
        
        auto duration = std::chrono::duration<double, std::milli>(end_time_ - start_time_);
        metrics_.function_name = name_;
        metrics_.execution_time_ms = duration.count();
        metrics_.success = true;
    }
}

inline PerformanceUtils::PerformanceMetrics PerformanceUtils::Profiler::get_metrics() const {
    return metrics_;
}

inline double PerformanceUtils::get_current_time_ms() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(now.time_since_epoch());
    return duration.count();
}

inline std::string PerformanceUtils::format_time(double milliseconds) {
    if (milliseconds < 1.0) {
        return std::to_string(milliseconds) + " ms";
    } else if (milliseconds < 1000.0) {
        return std::to_string(milliseconds) + " ms";
    } else {
        return std::to_string(milliseconds / 1000.0) + " s";
    }
}

inline std::string PerformanceUtils::format_memory(size_t bytes) {
    if (bytes < 1024) {
        return std::to_string(bytes) + " B";
    } else if (bytes < 1024 * 1024) {
        return std::to_string(bytes / 1024.0) + " KB";
    } else {
        return std::to_string(bytes / (1024.0 * 1024.0)) + " MB";
    }
}

inline std::string PerformanceUtils::format_throughput(double operations_per_second) {
    if (operations_per_second < 1000.0) {
        return std::to_string(operations_per_second) + " ops/s";
    } else if (operations_per_second < 1000000.0) {
        return std::to_string(operations_per_second / 1000.0) + " K ops/s";
    } else {
        return std::to_string(operations_per_second / 1000000.0) + " M ops/s";
    }
}

} // namespace utils
} // namespace r3m 