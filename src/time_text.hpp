// The text of the {time} and {utc} fields (docs/spec.md §6.1).
#pragma once

#include <chrono>
#include <string>

namespace cpm::log::detail {

// Local date and time: 2026-10-26 14:03:07.123
void append_local_time(std::string& out, std::chrono::system_clock::time_point time);

// UTC in ISO 8601: 2026-10-26T12:03:07.123Z
void append_utc_time(std::string& out, std::chrono::system_clock::time_point time);

}  // namespace cpm::log::detail
