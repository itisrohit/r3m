#include "r3m/api/jobs/job_manager.hpp"
#include <random>
#include <algorithm>
#include <chrono>

namespace r3m {
namespace api {

std::string JobManager::create_job(const std::string& file_path) {
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    
    std::string job_id = generate_job_id();
    
    ProcessingJob job;
    job.job_id = job_id;
    job.file_path = file_path;
    job.completed = false;
    job.created_at = std::chrono::system_clock::now();
    
    jobs_[job_id] = job;
    
    return job_id;
}

bool JobManager::complete_job(const std::string& job_id, const core::DocumentResult& result) {
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    
    auto it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        return false;
    }
    
    it->second.completed = true;
    it->second.result = result;
    it->second.completed_at = std::chrono::system_clock::now();
    
    return true;
}

bool JobManager::get_job(const std::string& job_id, ProcessingJob& job) const {
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    
    auto it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        return false;
    }
    
    job = it->second;
    return true;
}

bool JobManager::remove_job(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    
    auto it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        return false;
    }
    
    jobs_.erase(it);
    return true;
}

bool JobManager::is_job_completed(const std::string& job_id) const {
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    
    auto it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        return false;
    }
    
    return it->second.completed;
}

std::chrono::milliseconds JobManager::get_job_duration(const std::string& job_id) const {
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    
    auto it = jobs_.find(job_id);
    if (it == jobs_.end()) {
        return std::chrono::milliseconds(0);
    }
    
    const auto& job = it->second;
    if (!job.completed) {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - job.created_at);
    } else {
        return std::chrono::duration_cast<std::chrono::milliseconds>(job.completed_at - job.created_at);
    }
}

void JobManager::cleanup_old_jobs(std::chrono::hours max_age) {
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    
    auto now = std::chrono::system_clock::now();
    
    // Use erase-remove idiom for unordered_map
    for (auto it = jobs_.begin(); it != jobs_.end();) {
        if ((now - it->second.created_at) > max_age) {
            it = jobs_.erase(it);
        } else {
            ++it;
        }
    }
}

size_t JobManager::get_active_job_count() const {
    std::lock_guard<std::mutex> lock(jobs_mutex_);
    return jobs_.size();
}

std::string JobManager::generate_job_id() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* hex_chars = "0123456789abcdef";
    
    std::string job_id;
    for (int i = 0; i < 32; ++i) {
        job_id += hex_chars[dis(gen)];
    }
    return job_id;
}

} // namespace api
} // namespace r3m 