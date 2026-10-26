// The one place that converts to local time, which only the platform knows.

#include <ctime>

#include "civil_time.hpp"

namespace cpm::log::detail {

CivilTime local_time(std::chrono::system_clock::time_point time) noexcept {
    using namespace std::chrono;
    const auto ms = floor<milliseconds>(time);
    const auto seconds_since_epoch = floor<seconds>(ms);
    const std::time_t t = static_cast<std::time_t>(seconds_since_epoch.time_since_epoch().count());
    std::tm local{};
#if defined(_WIN32)
    const bool converted = localtime_s(&local, &t) == 0;
#else
    const bool converted = localtime_r(&t, &local) != nullptr;
#endif
    if (!converted) return utc_time(time);
    return {local.tm_year + 1900,
            static_cast<unsigned>(local.tm_mon + 1),
            static_cast<unsigned>(local.tm_mday),
            static_cast<unsigned>(local.tm_hour),
            static_cast<unsigned>(local.tm_min),
            static_cast<unsigned>(local.tm_sec),
            static_cast<unsigned>((ms - seconds_since_epoch).count())};
}

}  // namespace cpm::log::detail
