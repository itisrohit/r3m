#include "r3m/api/routes/response_handler/response_handler.hpp"
#include "r3m/api/routes/json_utils/json_utils.hpp"
#include <random>

namespace r3m {
namespace api {
namespace response_handler {

std::string create_response(bool success, const std::string& message, const std::string& data) {
#ifdef R3M_JSON_ENABLED
    json response;
    response["success"] = success;
    response["message"] = message;
    if (!data.empty()) {
        response["data"] = json::parse(data);
    }
    return response.dump();
#else
    // Simple text response if JSON not available
    if (data.empty()) {
        return "{\"success\":" + std::string(success ? "true" : "false") + 
               ",\"message\":\"" + message + "\"}";
    } else {
        return "{\"success\":" + std::string(success ? "true" : "false") + 
               ",\"message\":\"" + message + "\",\"data\":" + data + "}";
    }
#endif
}

std::string generate_job_id() {
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

} // namespace response_handler
} // namespace api
} // namespace r3m 