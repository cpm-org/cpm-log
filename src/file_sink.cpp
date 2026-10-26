#include <cpm/log/file_sink.hpp>

#include <utility>

#include "log_file.hpp"

namespace cpm::log {

FileSink::FileSink(std::filesystem::path path, FileOptions options, std::shared_ptr<const Layout> layout)
    : TextSink(std::move(layout)),
      path_(std::move(path)),
      options_(options),
      out_(detail::open_log_file(path_, options_.append)) {}

void FileSink::write_line(std::string_view line, const Record& record) {
    detail::write_log_line(out_, line, path_);
    if (record.level >= options_.flush_level) detail::flush_log_file(out_, path_);
}

void FileSink::flush_lines() { detail::flush_log_file(out_, path_); }

}  // namespace cpm::log
