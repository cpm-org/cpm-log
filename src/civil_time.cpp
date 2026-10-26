#include "civil_time.hpp"

namespace cpm::log::detail {

CivilTime utc_time(std::chrono::system_clock::time_point time) noexcept {
    using namespace std::chrono;
    const auto ms = floor<milliseconds>(time);
    const auto day = floor<days>(ms);
    const year_month_day date{day};
    const hh_mm_ss<milliseconds> clock{ms - day};
    return {static_cast<int>(date.year()),
            static_cast<unsigned>(date.month()),
            static_cast<unsigned>(date.day()),
            static_cast<unsigned>(clock.hours().count()),
            static_cast<unsigned>(clock.minutes().count()),
            static_cast<unsigned>(clock.seconds().count()),
            static_cast<unsigned>(clock.subseconds().count())};
}

}  // namespace cpm::log::detail
