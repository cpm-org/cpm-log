#include <cpm/log/memory_sink.hpp>

#include <utility>

namespace cpm::log {

MemorySink::MemorySink(std::shared_ptr<const Layout> layout) : TextSink(std::move(layout)) {}

std::vector<std::string> MemorySink::lines() const {
    const std::lock_guard lock(mutex_);
    return lines_;
}

void MemorySink::clear() {
    const std::lock_guard lock(mutex_);
    lines_.clear();
}

void MemorySink::write_line(std::string_view line, const Record&) {
    const std::lock_guard lock(mutex_);
    lines_.emplace_back(line.substr(0, line.size() - 1));
}

}  // namespace cpm::log
