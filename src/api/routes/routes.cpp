#include "r3m/api/routes/routes.hpp"
#include "r3m/api/routes/json_utils/json_utils.hpp"
#include "r3m/api/routes/response_handler/response_handler.hpp"
#include "r3m/api/routes/serialization/serializer.hpp"
#include "r3m/api/routes/route_handlers/route_handlers.hpp"
#include "r3m/utils/text_utils.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

namespace r3m {
namespace api {

Routes::Routes(std::shared_ptr<core::DocumentProcessor> processor, 
               const std::unordered_map<std::string, std::string>& config)
    : processor_(processor), config_(config) {
}

#ifdef R3M_HTTP_ENABLED

crow::response Routes::handle_health_check() {
    return route_handlers::handle_health_check();
}

crow::response Routes::handle_process_document(const crow::request& req) {
    return route_handlers::handle_process_document(req, processor_);
}

crow::response Routes::handle_process_batch(const crow::request& req) {
    return route_handlers::handle_process_batch(req, processor_);
}

crow::response Routes::handle_chunk_document(const crow::request& req) {
    return route_handlers::handle_chunk_document(req, processor_);
}

crow::response Routes::handle_job_status(const std::string& job_id) {
    return route_handlers::handle_job_status(job_id);
}

crow::response Routes::handle_system_info() {
    return route_handlers::handle_system_info(processor_, config_);
}

crow::response Routes::handle_metrics() {
    return route_handlers::handle_metrics(processor_);
}

#endif


} // namespace api
} // namespace r3m 