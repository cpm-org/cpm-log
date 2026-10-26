// What sinks receive (docs/spec.md §4.3).
#pragma once

#include <cpm/log/level.hpp>

#include <chrono>
#include <cstdint>
#include <source_location>
#include <string_view>

namespace cpm::log {

// One log message. The views are valid only during the call to the sink.
struct Record {
    std::chrono::system_clock::time_point time{};
    Level level = Level::info;
    std::string_view logger;   // the logger's name
    std::string_view message;  // formatted
    std::source_location where{};
    std::uint64_t thread = 0;  // see current_thread_number()
};

// The calling thread's number: 1 for the first thread that asks (or logs), 2 for the next, and so on. Stable for the
// life of the thread.
std::uint64_t current_thread_number() noexcept;

}  // namespace cpm::log
