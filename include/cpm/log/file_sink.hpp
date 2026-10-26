// Logging to files, and to files that roll over at a size (docs/spec.md §5.2, §5.3).
#pragma once

#include <cpm/log/sink.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>

namespace cpm::log {

struct FileOptions {
    bool append = true;                  // false: start the file empty
    Level flush_level = Level::warning;  // flush after records at this level and above
};

// Writes lines to a file, creating its parent directories. Throws cpm::log::Error from the constructor when the file
// can't be opened. Flushes after records at flush_level and above, on flush(), and when destroyed.
class FileSink final : public TextSink {
public:
    explicit FileSink(std::filesystem::path path, FileOptions options = {},
                      std::shared_ptr<const Layout> layout = nullptr);

    const std::filesystem::path& path() const noexcept { return path_; }

protected:
    void write_line(std::string_view line, const Record& record) override;
    void flush_lines() override;

private:
    std::filesystem::path path_;
    FileOptions options_;
    std::ofstream out_;
};

struct RotationOptions {
    std::uintmax_t max_bytes = 10 * 1024 * 1024;  // must be above 0
    unsigned max_files = 5;                       // rotated files kept beside the current one; 0: truncate instead
    Level flush_level = Level::warning;
};

// A FileSink that rotates before a line would make the file larger than max_bytes: server.log becomes server.1.log,
// server.1.log becomes server.2.log, and so on up to max_files, and server.log starts empty. It appends to an existing
// file, counting its size. A line longer than max_bytes still goes in, to a file of its own.
class RotatingFileSink final : public TextSink {
public:
    explicit RotatingFileSink(std::filesystem::path path, RotationOptions options = {},
                              std::shared_ptr<const Layout> layout = nullptr);

    const std::filesystem::path& path() const noexcept { return path_; }

protected:
    void write_line(std::string_view line, const Record& record) override;
    void flush_lines() override;

private:
    void open(bool append);

    std::filesystem::path path_;
    RotationOptions options_;
    std::ofstream out_;
    std::uintmax_t size_ = 0;
};

}  // namespace cpm::log
