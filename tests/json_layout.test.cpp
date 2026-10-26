// The JSON layout (docs/spec.md §6.2), read back with cpm-json.

#include <cpm/json.hpp>
#include <cpm/log.hpp>
#include <cpm/test.hpp>

#include <string>
#include <vector>

#include "support/helpers.hpp"

namespace json = cpm::json;
namespace logging = cpm::log;
using logging::Level;

namespace {

json::Value line_of(const logging::Record& record) {
    std::string out;
    logging::JsonLayout().format(record, out);
    INFO("line: {}", out);
    CHECK(out.find('\n') == std::string::npos);
    return json::parse(out);
}

}  // namespace

TEST_CASE("a record is one JSON object with its members in order") {
    const logging::Record r = support::record(Level::error, "lost 3 connections");
    const json::Value v = line_of(r);
    std::vector<std::string> keys;
    for (const json::Member& member : v.as_object()) keys.push_back(member.key());
    CHECK(keys == (std::vector<std::string>{"time", "level", "logger", "message", "file", "line", "function", "thread"}));
    CHECK(v.at("time").as_string() == "2025-10-26T14:03:07.123Z");
    CHECK(v.at("level").as_string() == "error");
    CHECK(v.at("logger").as_string() == "test");
    CHECK(v.at("message").as_string() == "lost 3 connections");
    CHECK(v.at("file").as_string() == r.where.file_name());
    CHECK(v.at("line").get<unsigned>() == r.where.line());
    CHECK(v.at("function").as_string() == r.where.function_name());
    CHECK(v.at("thread").get<int>() == 7);
}

TEST_CASE("messages are escaped") {
    const std::string message = std::string("quote \" backslash \\ tab \t newline \n bell \x07 nul ") + '\0' + " end";
    CHECK(line_of(support::record(Level::info, message)).at("message").as_string() == message);
}

TEST_CASE("bytes that aren't UTF-8 become U+FFFD") {
    const json::Value v = line_of(support::record(Level::info, "bad \xFF byte, cut \xE2\x82, fine \xE2\x82\xAC"));
    CHECK(v.at("message").as_string() == "bad \xEF\xBF\xBD byte, cut \xEF\xBF\xBD\xEF\xBF\xBD, fine \xE2\x82\xAC");
}
