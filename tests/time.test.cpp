// Time (docs/spec.md §7): UTC by calendar arithmetic, local time from the platform, and their text.

#include <cpm/test.hpp>

#include <chrono>
#include <string>

#include "civil_time.hpp"
#include "time_text.hpp"

namespace detail = cpm::log::detail;
using std::chrono::milliseconds;
using std::chrono::system_clock;

namespace {

std::string utc(long long ms) {
    std::string text;
    detail::append_utc_time(text, system_clock::time_point(milliseconds(ms)));
    return text;
}

}  // namespace

TEST_CASE("UTC dates and times, checked against Python's datetime") {
    CHECK(utc(0) == "1970-01-01T00:00:00.000Z");
    CHECK(utc(-1) == "1969-12-31T23:59:59.999Z");  // before 1970, truncated towards the past
    CHECK(utc(951782400000) == "2000-02-29T00:00:00.000Z");   // a leap year divisible by 400
    CHECK(utc(1709164800000) == "2024-02-29T00:00:00.000Z");
    CHECK(utc(4107542400000) == "2100-03-01T00:00:00.000Z");  // 2100 isn't a leap year
    CHECK(utc(-2203891200000) == "1900-03-01T00:00:00.000Z");  // nor is 1900
    CHECK(utc(1761487387123) == "2025-10-26T14:03:07.123Z");
    CHECK(utc(1234567890123) == "2009-02-13T23:31:30.123Z");
}

TEST_CASE("UTC fields") {
    const detail::CivilTime t = detail::utc_time(system_clock::time_point(milliseconds(1761487387123)));
    CHECK(t == (detail::CivilTime{2025, 10, 26, 14, 3, 7, 123}));
}

TEST_CASE("sub-millisecond time is truncated") {
    const auto time = system_clock::time_point(milliseconds(1761487387123)) + std::chrono::microseconds(999);
    CHECK(detail::utc_time(time).millisecond == 123);
}

TEST_CASE("local time has the same shape, and the same seconds as UTC") {
    const auto time = system_clock::time_point(milliseconds(1761487387123));
    std::string text;
    detail::append_local_time(text, time);
    INFO("local time: {}", text);
    REQUIRE(text.size() == 23);  // 2025-10-26 14:03:07.123
    CHECK(text[4] == '-');
    CHECK(text[10] == ' ');
    CHECK(text[19] == '.');
    CHECK(text.substr(20) == "123");
    // Time zones are whole hours or half hours (or 45 minutes) from UTC; the seconds never differ.
    CHECK(text.substr(17, 2) == "07");
    const detail::CivilTime local = detail::local_time(time);
    CHECK(local.second == 7);
    CHECK(local.millisecond == 123);
}
