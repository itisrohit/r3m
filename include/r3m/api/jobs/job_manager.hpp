#pragma once

#include "r3m/core/document_processor.hpp"
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>

namespace r3m {
namespace api {

struct ProcessingJob {
    std::string job_id;
    std::string file_path;
    bool completed = false;
    core::DocumentResult result;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point completed_at;
};

class JobManager {
public:
    JobManager() = default;
    ~JobManager() = default;

    // Job management
    std::string create_job(const std::string& file_path);
    bool complete_job(const std::string& job_id, const core::DocumentResult& result);
    bool get_job(const std::string& job_id, ProcessingJob& job) const;
    bool remove_job(const std::string& job_id);
    
    // Job status
    bool is_job_completed(const std::string& job_id) const;
    std::chrono::milliseconds get_job_duration(const std::string& job_id) const;
    
    // Cleanup
    void cleanup_old_jobs(std::chrono::hours max_age = std::chrono::hours(24));
    size_t get_active_job_count() const;

private:
    mutable std::mutex jobs_mutex_;
    std::unordered_map<std::string, ProcessingJob> jobs_;
    
    std::string generate_job_id() const;
};

} // namespace api
} // namespace r3m 