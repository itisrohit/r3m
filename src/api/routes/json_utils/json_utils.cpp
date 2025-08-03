#include "r3m/api/routes/json_utils/json_utils.hpp"
#include <cstdio>

namespace r3m {
namespace api {
namespace json_utils {

std::string escape_json_string(const std::string& input) {
    std::string output;
    output.reserve(input.length() * 2); // Reserve space for potential escaping
    
    for (unsigned char c : input) {
        switch (c) {
            case '"':  output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default:
                if (c < 32) {
                    // Control characters
                    char hex[7];
                    snprintf(hex, sizeof(hex), "\\u%04x", c);
                    output += hex;
                } else {
                    output += static_cast<char>(c);
                }
                break;
        }
    }
    return output;
}

} // namespace json_utils
} // namespace api
} // namespace r3m 