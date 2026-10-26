// Opening log files and writing to them, shared by FileSink and RotatingFileSink.
#pragma once

#include <filesystem>
#include <fstream>
#include <string_view>

namespace cpm::log::detail {

// Opens `path` for writing, appending or truncating, after creating its parent directories. Throws cpm::log::Error
// when that fails.
std::ofstream open_log_file(const std::filesystem::path& path, bool append);

// Writes `line`; throws cpm::log::Error when the stream fails.
void write_log_line(std::ofstream& out, std::string_view line, const std::filesystem::path& path);

// Flushes; throws cpm::log::Error when the stream fails.
void flush_log_file(std::ofstream& out, const std::filesystem::path& path);

}  // namespace cpm::log::detail
