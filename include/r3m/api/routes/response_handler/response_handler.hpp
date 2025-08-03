#pragma once

#include <string>
#include <random>

#ifdef R3M_JSON_ENABLED
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#endif

namespace r3m {
namespace api {
namespace response_handler {

/**
 * @brief Create a JSON response with success status and message
 * @param success Whether the operation was successful
 * @param message Response message
 * @param data Optional data payload
 * @return JSON response string
 */
std::string create_response(bool success, const std::string& message, const std::string& data = "");

/**
 * @brief Generate a unique job ID for tracking operations
 * @return 32-character hexadecimal job ID
 */
std::string generate_job_id();

} // namespace response_handler
} // namespace api
} // namespace r3m 