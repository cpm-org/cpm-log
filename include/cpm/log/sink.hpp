// Where records go (docs/spec.md §5).
#pragma once

#include <cpm/log/layout.hpp>
#include <cpm/log/level.hpp>
#include <cpm/log/record.hpp>

#include <atomic>
#include <memory>
#include <mutex>
#include <string_view>

namespace cpm::log {

// Receives records. write() and flush() may be called from many threads at once; implementations make that safe.
class Sink {
public:
    virtual ~Sink() = default;
    virtual void write(const Record& record) = 0;
    virtual void flush() = 0;

protected:
    Sink() = default;
    Sink(const Sink&) = default;
    Sink& operator=(const Sink&) = default;
};

// The base of the sinks that write lines of text. It keeps a level of its own, turns each record into a line with its
// layout, and hands the line to the derived sink under a lock, so lines from different threads never interleave.
class TextSink : public Sink {
public:
    TextSink(const TextSink&) = delete;
    TextSink& operator=(const TextSink&) = delete;

    void write(const Record& record) final;
    void flush() final;

    // Records below this level are dropped. Default: trace.
    Level level() const noexcept { return level_.load(std::memory_order_relaxed); }
    void set_level(Level level) noexcept { level_.store(level, std::memory_order_relaxed); }

protected:
    // nullptr: default_layout().
    explicit TextSink(std::shared_ptr<const Layout> layout);

    // Writes `line`, which ends in '\n'. Called under the sink's lock.
    virtual void write_line(std::string_view line, const Record& record) = 0;
    // Flushes what write_line wrote. Called under the sink's lock.
    virtual void flush_lines() = 0;

private:
    std::shared_ptr<const Layout> layout_;
    std::atomic<Level> level_{Level::trace};
    std::mutex mutex_;
};

}  // namespace cpm::log
