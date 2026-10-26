// A time point as a calendar date and time of day (docs/spec.md §7).
#pragma once

#include <chrono>

namespace cpm::log::detail {

struct CivilTime {
    int year = 1970;
    unsigned month = 1;   // 1 to 12
    unsigned day = 1;     // 1 to 31
    unsigned hour = 0;
    unsigned minute = 0;
    unsigned second = 0;
    unsigned millisecond = 0;

    friend bool operator==(const CivilTime&, const CivilTime&) = default;
};

// The UTC date and time of `time`, truncated to milliseconds (towards the past, also before 1970). Pure arithmetic on
// the days since 1970, so it is the same on every platform.
CivilTime utc_time(std::chrono::system_clock::time_point time) noexcept;

// The local date and time of `time`, truncated to milliseconds, from the platform's thread-safe conversion. Falls back
// to UTC if the platform can't convert it.
CivilTime local_time(std::chrono::system_clock::time_point time) noexcept;

}  // namespace cpm::log::detail
