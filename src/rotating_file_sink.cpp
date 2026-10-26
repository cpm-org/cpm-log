#include <cpm/log/error.hpp>
#include <cpm/log/file_sink.hpp>

#include <system_error>
#include <utility>

#include "log_file.hpp"
#include "rotation.hpp"

namespace cpm::log {

RotatingFileSink::RotatingFileSink(std::filesystem::path path, RotationOptions options,
                                   std::shared_ptr<const Layout> layout)
    : TextSink(std::move(layout)), path_(std::move(path)), options_(options) {
    if (options_.max_bytes == 0) throw Error("a rotating log file needs max_bytes above 0");
    open(true);
}

void RotatingFileSink::open(bool append) {
    out_ = detail::open_log_file(path_, append);
    std::error_code ec;
    const std::uintmax_t size = append ? std::filesystem::file_size(path_, ec) : 0;
    size_ = ec ? 0 : size;
}

void RotatingFileSink::write_line(std::string_view line, const Record& record) {
    if (size_ > 0 && size_ + line.size() > options_.max_bytes) {
        out_.close();
        if (options_.max_files == 0) {
            open(false);
        } else {
            try {
                detail::rotate_files(path_, options_.max_files);
            } catch (...) {
                open(true);  // keep logging to the current file
                throw;
            }
            open(false);
        }
    }
    detail::write_log_line(out_, line, path_);
    size_ += line.size();
    if (record.level >= options_.flush_level) detail::flush_log_file(out_, path_);
}

void RotatingFileSink::flush_lines() { detail::flush_log_file(out_, path_); }

}  // namespace cpm::log
