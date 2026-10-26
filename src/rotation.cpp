#include "rotation.hpp"

#include <cpm/log/error.hpp>

#include <format>
#include <string>
#include <system_error>

namespace cpm::log::detail {

std::filesystem::path rotated_path(const std::filesystem::path& path, unsigned number) {
    std::filesystem::path name = path.stem();
    name += "." + std::to_string(number);
    name += path.extension();
    return path.parent_path() / name;
}

void rotate_files(const std::filesystem::path& path, unsigned max_files) {
    std::error_code ec;
    std::filesystem::remove(rotated_path(path, max_files), ec);  // the oldest; it may not exist
    for (unsigned number = max_files; number > 1; --number) {
        const std::filesystem::path from = rotated_path(path, number - 1);
        if (!std::filesystem::exists(from, ec)) continue;
        std::filesystem::rename(from, rotated_path(path, number), ec);
        if (ec) throw Error(std::format("can't rotate the log file {}: {}", from.string(), ec.message()));
    }
    std::filesystem::rename(path, rotated_path(path, 1), ec);
    if (ec) throw Error(std::format("can't rotate the log file {}: {}", path.string(), ec.message()));
}

}  // namespace cpm::log::detail
