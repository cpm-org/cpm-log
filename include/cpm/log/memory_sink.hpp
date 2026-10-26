// Logging to memory, for tests (docs/spec.md §5.1).
#pragma once

#include <cpm/log/sink.hpp>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace cpm::log {

// Keeps the lines it is given, without their line breaks.
class MemorySink final : public TextSink {
public:
    explicit MemorySink(std::shared_ptr<const Layout> layout = nullptr);

    // A copy of the lines so far.
    std::vector<std::string> lines() const;
    void clear();

protected:
    void write_line(std::string_view line, const Record& record) override;
    void flush_lines() override {}

private:
    mutable std::mutex mutex_;  // lines() and clear() run outside the TextSink lock
    std::vector<std::string> lines_;
};

}  // namespace cpm::log
