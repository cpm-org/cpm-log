// Shared by the tests: records with a known time, temporary directories and file contents.
#pragma once

#include <cpm/log.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <random>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>

namespace support {

// 2025-10-26T14:03:07.123Z
inline constexpr std::chrono::milliseconds known_time{1761487387123};

// A record at a known time, from logger "test" on thread 7, made where it is called.
inline cpm::log::Record record(cpm::log::Level level, std::string_view message,
                               std::source_location where = std::source_location::current()) {
    return {std::chrono::system_clock::time_point(known_time), level, "test", message, where, 7};
}

// A fresh empty directory, removed with everything in it when the object goes away.
class TempDir {
public:
    TempDir() {
        std::random_device random;
        path_ = std::filesystem::temp_directory_path() / ("cpm-log-test-" + std::to_string(random()));
        std::filesystem::create_directories(path_);
    }
    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;
    ~TempDir() {
        std::error_code ec;
        std::filesystem::remove_all(path_, ec);
    }

    const std::filesystem::path& path() const noexcept { return path_; }

private:
    std::filesystem::path path_;
};

// The whole file, or "" when it doesn't exist.
inline std::string read_file(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream text;
    text << in.rdbuf();
    return text.str();
}

// A layout of just the message, for tests about where lines go rather than how they look.
inline std::shared_ptr<const cpm::log::Layout> message_layout() {
    return std::make_shared<cpm::log::PatternLayout>("{message}");
}

}  // namespace support
