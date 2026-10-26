#include <cpm/log/record.hpp>

#include <atomic>

namespace cpm::log {

std::uint64_t current_thread_number() noexcept {
    static std::atomic<std::uint64_t> next{1};
    thread_local const std::uint64_t number = next.fetch_add(1, std::memory_order_relaxed);
    return number;
}

}  // namespace cpm::log
