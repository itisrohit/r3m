#pragma once

#include <string>

namespace r3m {
namespace api {
namespace json_utils {

/**
 * @brief Escape special characters in a string for JSON output
 * @param input Input string to escape
 * @return Escaped string safe for JSON
 */
std::string escape_json_string(const std::string& input);

/**
 * @brief JSON fallback implementation when nlohmann/json is not available
 */
#ifndef R3M_JSON_ENABLED
namespace json {
    class basic_json {
    public:
        std::string dump() const { return "{}"; }
        void push_back(const basic_json&) {}
        basic_json& operator[](const std::string&) { return *this; }
        basic_json& operator[](int) { return *this; }
        template<typename T>
        basic_json& operator=(const T&) { return *this; }
        bool has(const std::string&) const { return false; }
        std::string s() const { return ""; }
        bool b() const { return false; }
        int i() const { return 0; }
        double d() const { return 0.0; }
    };
    using json = basic_json;
    inline json array() { return json(); }
    inline json parse(const std::string&) { return json(); }
}
#endif

} // namespace json_utils
} // namespace api
} // namespace r3m 