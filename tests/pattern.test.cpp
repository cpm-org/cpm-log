// Pattern layouts (docs/spec.md §6.1): parsing, every field, specifications and errors.

#include <cpm/log.hpp>
#include <cpm/test.hpp>

#include <string>

#include "pattern.hpp"
#include "support/helpers.hpp"

namespace logging = cpm::log;
using logging::Level;
using logging::detail::Field;

namespace {

std::string format(std::string_view pattern, const logging::Record& record) {
    std::string out;
    logging::PatternLayout(pattern).format(record, out);
    return out;
}

}  // namespace

TEST_CASE("patterns parse into text and fields") {
    const auto segments = logging::detail::parse_pattern("{utc} x {{y}} {level:<8}!");
    REQUIRE(segments.size() == 4);
    CHECK(segments[0].is_field);
    CHECK(segments[0].field == Field::utc);
    CHECK(!segments[1].is_field);
    CHECK(segments[1].text == " x {y} ");
    CHECK(segments[2].field == Field::level);
    CHECK(segments[2].format == "{:<8}");
    CHECK(segments[3].text == "!");
    CHECK(logging::detail::parse_pattern("").empty());
}

TEST_CASE("every field") {
    const logging::Record r = support::record(Level::warning, "disk at 91%");
    CHECK(format("{utc}", r) == "2025-10-26T14:03:07.123Z");
    CHECK(format("{level}", r) == "warning");
    CHECK(format("{logger}", r) == "test");
    CHECK(format("{message}", r) == "disk at 91%");
    CHECK(format("{thread}", r) == "7");
    CHECK(format("{line}", r) == std::to_string(r.where.line()));
    CHECK(format("{path}", r) == r.where.file_name());
    CHECK(format("{file}", r) == "pattern.test.cpp");
    CHECK(format("{function}", r) == r.where.function_name());
    CHECK(format("{time}", r).size() == 23);
    CHECK(format("[{level}] {message} ({file}:{line})", r) ==
          "[warning] disk at 91% (pattern.test.cpp:" + std::to_string(r.where.line()) + ")");
}

TEST_CASE("fields take std::format specifications") {
    const logging::Record r = support::record(Level::info, "m");
    CHECK(format("{level:<8}|", r) == "info    |");
    CHECK(format("{level:>8}|", r) == "    info|");
    CHECK(format("{level:^9}|", r) == "  info   |");
    CHECK(format("{thread:03}", r) == "007");
    CHECK(format("{logger:.2}", r) == "te");
    CHECK(format("{message:*<3}", r) == "m**");
}

TEST_CASE("the default pattern") {
    const logging::Record r = support::record(Level::warning, "slow response");
    std::string out;
    logging::default_layout()->format(r, out);
    REQUIRE(out.size() > 24);
    CHECK(out.substr(23) == " warning  test: slow response");
    CHECK(logging::default_layout() == logging::default_layout());  // one shared layout
}

TEST_CASE("messages are written as they are") {
    const logging::Record r = support::record(Level::info, "{braces} \"quotes\"\n\xFF");
    CHECK(format("{message}", r) == "{braces} \"quotes\"\n\xFF");
}

TEST_CASE("patterns that aren't valid say where") {
    CHECK_THROWS_WITH(logging::PatternLayout("{nope}"),
                      "the log pattern \"{nope}\" isn't valid: an unknown field {nope} at position 1");
    CHECK_THROWS_WITH(logging::PatternLayout("abc {level"),
                      "the log pattern \"abc {level\" isn't valid: a '{' that isn't closed at position 5");
    CHECK_THROWS_WITH(logging::PatternLayout("a } b"),
                      "the log pattern \"a } b\" isn't valid: a '}' that closes nothing (write }} for a brace) at position 3");
    CHECK_THROWS_WITH(logging::PatternLayout("x {time:%H}"),
                      "the log pattern \"x {time:%H}\" isn't valid: {time} takes no format specification at position 3");
    CHECK_THROWS_WITH(logging::PatternLayout("{line:s}"),
                      "the log pattern \"{line:s}\" isn't valid: {line} can't take the format specification \"s\" at position 1");
    CHECK_THROWS_AS(logging::PatternLayout("{level:d}"), logging::Error);
    CHECK_THROWS_AS(logging::PatternLayout("{}"), logging::Error);
    CHECK_THROWS_AS(logging::PatternLayout("{Level}"), logging::Error);  // names are lowercase
}
