#include "time_text.hpp"

#include <format>
#include <iterator>

#include "civil_time.hpp"

namespace cpm::log::detail {

namespace {

void append(std::string& out, const CivilTime& t, char separator, const char* suffix) {
    std::format_to(std::back_inserter(out), "{:04}-{:02}-{:02}{}{:02}:{:02}:{:02}.{:03}{}", t.year, t.month, t.day,
                   separator, t.hour, t.minute, t.second, t.millisecond, suffix);
}

}  // namespace

void append_local_time(std::string& out, std::chrono::system_clock::time_point time) {
    append(out, local_time(time), ' ', "");
}

void append_utc_time(std::string& out, std::chrono::system_clock::time_point time) {
    append(out, utc_time(time), 'T', "Z");
}

}  // namespace cpm::log::detail
