// The process-wide logger and the free functions that log to it (docs/spec.md §4.5).
#pragma once

#include <cpm/log/logger.hpp>

#include <utility>

namespace cpm::log {

// Named "main", at level info, with one ConsoleSink on standard error and automatic colors. Configure it with
// set_level and set_sinks. It is never destroyed, so code that runs while statics are destroyed can still log.
Logger& default_logger();

template <class... Args>
void log(Level level, FormatFor<Args...> format, Args&&... args) noexcept {
    default_logger().log(level, format, std::forward<Args>(args)...);
}
template <class... Args>
void trace(FormatFor<Args...> format, Args&&... args) noexcept {
    default_logger().log(Level::trace, format, std::forward<Args>(args)...);
}
template <class... Args>
void debug(FormatFor<Args...> format, Args&&... args) noexcept {
    default_logger().log(Level::debug, format, std::forward<Args>(args)...);
}
template <class... Args>
void info(FormatFor<Args...> format, Args&&... args) noexcept {
    default_logger().log(Level::info, format, std::forward<Args>(args)...);
}
template <class... Args>
void warning(FormatFor<Args...> format, Args&&... args) noexcept {
    default_logger().log(Level::warning, format, std::forward<Args>(args)...);
}
template <class... Args>
void error(FormatFor<Args...> format, Args&&... args) noexcept {
    default_logger().log(Level::error, format, std::forward<Args>(args)...);
}
template <class... Args>
void critical(FormatFor<Args...> format, Args&&... args) noexcept {
    default_logger().log(Level::critical, format, std::forward<Args>(args)...);
}

}  // namespace cpm::log
