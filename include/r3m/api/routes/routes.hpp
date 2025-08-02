#pragma once

#include "r3m/core/document_processor.hpp"
#include "r3m/chunking/chunk_models.hpp"
#include <string>
#include <memory>

#ifdef R3M_HTTP_ENABLED
#include <crow.h>
#endif

namespace r3m {
namespace api {

class Routes {
public:
    explicit Routes(std::shared_ptr<core::DocumentProcessor> processor);
    ~Routes() = default;

    // Route handlers
#ifdef R3M_HTTP_ENABLED
    crow::response handle_health_check();
    crow::response handle_process_document(const crow::request& req);
    crow::response handle_process_batch(const crow::request& req);
    crow::response handle_chunk_document(const crow::request& req);
    crow::response handle_job_status(const std::string& job_id);
    crow::response handle_system_info();
    crow::response handle_metrics();
#endif

private:
    std::shared_ptr<core::DocumentProcessor> processor_;
};

} // namespace api
} // namespace r3m 