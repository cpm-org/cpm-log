#include <cpm/log/sink.hpp>

#include <string>
#include <utility>

namespace cpm::log {

TextSink::TextSink(std::shared_ptr<const Layout> layout)
    : layout_(layout ? std::move(layout) : default_layout()) {}

void TextSink::write(const Record& record) {
    const Level threshold = level();
    if (threshold == Level::off || record.level < threshold) return;
    std::string line;
    layout_->format(record, line);  // outside the lock: layouts are immutable
    line += '\n';
    const std::lock_guard lock(mutex_);
    write_line(line, record);
}

void TextSink::flush() {
    const std::lock_guard lock(mutex_);
    flush_lines();
}

}  // namespace cpm::log
