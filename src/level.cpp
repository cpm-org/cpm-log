#include <cpm/log/level.hpp>

#include <algorithm>

namespace cpm::log {

namespace {

// Whether `text` is `name` in any case; `name` is lowercase ASCII.
bool names(std::string_view text, std::string_view name) noexcept {
    return std::equal(text.begin(), text.end(), name.begin(), name.end(), [](char t, char n) {
        return (t >= 'A' && t <= 'Z' ? static_cast<char>(t - 'A' + 'a') : t) == n;
    });
}

}  // namespace

std::string_view to_string(Level level) noexcept {
    switch (level) {
        case Level::trace: return "trace";
        case Level::debug: return "debug";
        case Level::info: return "info";
        case Level::warning: return "warning";
        case Level::error: return "error";
        case Level::critical: return "critical";
        case Level::off: return "off";
    }
    return "?";
}

std::optional<Level> parse_level(std::string_view text) noexcept {
    if (names(text, "warn")) return Level::warning;
    for (Level level : {Level::trace, Level::debug, Level::info, Level::warning, Level::error, Level::critical,
                        Level::off}) {
        if (names(text, to_string(level))) return level;
    }
    return std::nullopt;
}

}  // namespace cpm::log
