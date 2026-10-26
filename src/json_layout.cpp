#include <cpm/log/layout.hpp>

#include <format>
#include <iterator>

#include "time_text.hpp"
#include "utf8.hpp"

namespace cpm::log {

namespace {

// Appends `text` as a JSON string. Bytes that don't start a well-formed UTF-8 sequence become U+FFFD, one each.
void append_json_string(std::string& out, std::string_view text) {
    out += '"';
    for (std::size_t i = 0; i < text.size();) {
        const auto byte = static_cast<unsigned char>(text[i]);
        if (byte >= 0x80) {
            const std::size_t length = detail::utf8::sequence_length(text, i);
            if (length == 0) {
                out += "\xEF\xBF\xBD";
                ++i;
            } else {
                out.append(text, i, length);
                i += length;
            }
            continue;
        }
        switch (byte) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (byte < 0x20) std::format_to(std::back_inserter(out), "\\u{:04x}", static_cast<unsigned>(byte));
                else out += static_cast<char>(byte);
        }
        ++i;
    }
    out += '"';
}

}  // namespace

void JsonLayout::format(const Record& record, std::string& out) const {
    out += R"({"time":")";
    detail::append_utc_time(out, record.time);
    out += R"(","level":)";
    append_json_string(out, to_string(record.level));
    out += R"(,"logger":)";
    append_json_string(out, record.logger);
    out += R"(,"message":)";
    append_json_string(out, record.message);
    out += R"(,"file":)";
    append_json_string(out, record.where.file_name());
    std::format_to(std::back_inserter(out), R"(,"line":{},"function":)", record.where.line());
    append_json_string(out, record.where.function_name());
    std::format_to(std::back_inserter(out), R"(,"thread":{}}})", record.thread);
}

}  // namespace cpm::log
