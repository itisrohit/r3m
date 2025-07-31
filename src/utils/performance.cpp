#include "r3m/utils/performance.hpp"
#include <algorithm>
#include <numeric>
#include <fstream>
#include <cmath>
#include <thread>
#include <sys/resource.h>
#include <unistd.h>

namespace r3m {
namespace utils {

// BenchmarkRunner implementation
PerformanceUtils::BenchmarkRunner::BenchmarkRunner(const std::string& name, size_t iterations)
    : name_(name), iterations_(iterations), warmup_iterations_(10), memory_tracking_(false) {}



void PerformanceUtils::BenchmarkRunner::set_warmup_iterations(size_t count) {
    warmup_iterations_ = count;
}

void PerformanceUtils::BenchmarkRunner::set_memory_tracking(bool enabled) {
    memory_tracking_ = enabled;
}

double PerformanceUtils::BenchmarkRunner::calculate_std_deviation(const std::vector<double>& times) {
    if (times.empty()) return 0.0;
    
    double mean = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
    double variance = 0.0;
    
    for (double time : times) {
        variance += std::pow(time - mean, 2);
    }
    variance /= times.size();
    
    return std::sqrt(variance);
}

size_t PerformanceUtils::BenchmarkRunner::get_current_memory_usage() {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss * 1024; // Convert KB to bytes
    }
    return 0;
}

// PerformanceMonitor implementation
PerformanceUtils::PerformanceMonitor::PerformanceMonitor() : is_monitoring_(false) {}

void PerformanceUtils::PerformanceMonitor::start_monitoring() {
    start_time_ = std::chrono::high_resolution_clock::now();
    is_monitoring_ = true;
    metrics_.clear();
}

void PerformanceUtils::PerformanceMonitor::stop_monitoring() {
    is_monitoring_ = false;
}

void PerformanceUtils::PerformanceMonitor::add_metric(const std::string& name, double value) {
    if (is_monitoring_) {
        metrics_[name].push_back(value);
    }
}

void PerformanceUtils::PerformanceMonitor::add_metric(const std::string& name, size_t value) {
    if (is_monitoring_) {
        metrics_[name].push_back(static_cast<double>(value));
    }
}

std::unordered_map<std::string, std::vector<double>> PerformanceUtils::PerformanceMonitor::get_metrics() const {
    return metrics_;
}

void PerformanceUtils::PerformanceMonitor::print_summary() const {
    std::cout << "\n📊 PERFORMANCE MONITOR SUMMARY\n";
    std::cout << "================================\n";
    
    for (const auto& [name, values] : metrics_) {
        if (values.empty()) continue;
        
        double sum = std::accumulate(values.begin(), values.end(), 0.0);
        double avg = sum / values.size();
        double min = *std::min_element(values.begin(), values.end());
        double max = *std::max_element(values.begin(), values.end());
        
        std::cout << "📈 " << name << ":\n";
        std::cout << "   Count: " << values.size() << "\n";
        std::cout << "   Average: " << avg << "\n";
        std::cout << "   Min: " << min << "\n";
        std::cout << "   Max: " << max << "\n";
        std::cout << "   Total: " << sum << "\n\n";
    }
}

void PerformanceUtils::PerformanceMonitor::export_to_csv(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "❌ Failed to open file: " << filename << std::endl;
        return;
    }
    
    // Write header
    file << "Metric,Value,Timestamp\n";
    
    auto now = std::chrono::high_resolution_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    for (const auto& [name, values] : metrics_) {
        for (double value : values) {
            file << name << "," << value << "," << timestamp << "\n";
        }
    }
    
    file.close();
    std::cout << "✅ Performance data exported to: " << filename << std::endl;
}

// Static utility functions implementation
size_t PerformanceUtils::get_current_memory_usage() {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss * 1024; // Convert KB to bytes
    }
    return 0;
}

// Explicit template instantiations for common use cases
template PerformanceUtils::BenchmarkResults PerformanceUtils::BenchmarkRunner::run_benchmark<std::function<void()>>(std::function<void()>&& func);

} // namespace utils
} // namespace r3m 