// Logging to a std::ostream (docs/spec.md §5.1).
#pragma once

#include <cpm/log/sink.hpp>

#include <iosfwd>
#include <memory>

namespace cpm::log {

// Writes lines to a stream, flushed on flush(). Throws cpm::log::Error when the stream fails.
class StreamSink final : public TextSink {
public:
    explicit StreamSink(std::ostream& out, std::shared_ptr<const Layout> layout = nullptr);

protected:
    void write_line(std::string_view line, const Record& record) override;
    void flush_lines() override;

private:
    std::ostream& out_;
};

}  // namespace cpm::log
