#include "log_file.hpp"

#include <cpm/log/error.hpp>

#include <format>
#include <system_error>

namespace cpm::log::detail {

std::ofstream open_log_file(const std::filesystem::path& path, bool append) {
    std::error_code ec;
    if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream out(path, std::ios::binary | (append ? std::ios::app : std::ios::trunc));
    if (!out) {
        const std::string why = ec ? std::format(" ({})", ec.message()) : std::string();
        throw Error(std::format("can't open the log file {}{}", path.string(), why));
    }
    return out;
}

void write_log_line(std::ofstream& out, std::string_view line, const std::filesystem::path& path) {
    out.write(line.data(), static_cast<std::streamsize>(line.size()));
    if (!out) throw Error(std::format("writing to the log file {} failed", path.string()));
}

void flush_log_file(std::ofstream& out, const std::filesystem::path& path) {
    out.flush();
    if (!out) throw Error(std::format("flushing the log file {} failed", path.string()));
}

}  // namespace cpm::log::detail
