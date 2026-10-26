#include <cpm/log/error.hpp>
#include <cpm/log/stream_sink.hpp>

#include <ostream>
#include <utility>

namespace cpm::log {

StreamSink::StreamSink(std::ostream& out, std::shared_ptr<const Layout> layout)
    : TextSink(std::move(layout)), out_(out) {}

void StreamSink::write_line(std::string_view line, const Record&) {
    out_.write(line.data(), static_cast<std::streamsize>(line.size()));
    if (!out_) throw Error("writing a log line to the stream failed");
}

void StreamSink::flush_lines() {
    out_.flush();
    if (!out_) throw Error("flushing the log stream failed");
}

}  // namespace cpm::log
