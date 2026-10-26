// Rotated log files: their names, and moving them along (docs/spec.md §5.3).
#pragma once

#include <filesystem>

namespace cpm::log::detail {

// The name of rotated file `number`: the number goes before the extension, server.log -> server.2.log, and at the end
// of a name without one, server -> server.2.
std::filesystem::path rotated_path(const std::filesystem::path& path, unsigned number);

// Moves the files along, the current one (closed) included: deletes number max_files, renames each numbered file to
// the next number, and the current file to number 1. Throws cpm::log::Error when a rename fails.
void rotate_files(const std::filesystem::path& path, unsigned max_files);

}  // namespace cpm::log::detail
