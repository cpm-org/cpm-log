// Files and rotating files (docs/spec.md §5.2, §5.3).

#include <cpm/log.hpp>
#include <cpm/test.hpp>

#include <filesystem>
#include <string>

#include "rotation.hpp"
#include "support/helpers.hpp"

namespace logging = cpm::log;
namespace fs = std::filesystem;
using logging::Level;

namespace {

// Writes `message` (plus its line break) through `sink`.
template <class Sink>
void put(Sink& sink, std::string_view message, Level level = Level::info) {
    sink.write(support::record(level, message));
}

}  // namespace

TEST_CASE("file sinks append lines to a file, creating its directories") {
    support::TempDir dir;
    const fs::path path = dir.path() / "logs" / "deep" / "app.log";
    {
        logging::FileSink sink(path, {}, support::message_layout());
        put(sink, "one");
        put(sink, "two");
    }
    CHECK(support::read_file(path) == "one\ntwo\n");
    {
        logging::FileSink sink(path, {}, support::message_layout());
        put(sink, "three");
    }
    CHECK(support::read_file(path) == "one\ntwo\nthree\n");

    SUBCASE("or start it empty") {
        {
            logging::FileSink sink(path, {.append = false}, support::message_layout());
            put(sink, "fresh");
        }
        CHECK(support::read_file(path) == "fresh\n");
    }
}

TEST_CASE("file sinks flush at the flush level") {
    support::TempDir dir;
    const fs::path path = dir.path() / "app.log";
    logging::FileSink sink(path, {.flush_level = Level::error}, support::message_layout());
    put(sink, "early");
    put(sink, "failed", Level::error);
    CHECK(support::read_file(path) == "early\nfailed\n");  // the sink is still open
    put(sink, "later");
    sink.flush();
    CHECK(support::read_file(path) == "early\nfailed\nlater\n");
}

TEST_CASE("a file that can't be opened is an Error") {
    support::TempDir dir;
    CHECK_THROWS_AS(logging::FileSink(dir.path()), logging::Error);  // a directory
}

TEST_CASE("rotated files are numbered before the extension") {
    CHECK(logging::detail::rotated_path("logs/server.log", 1) == fs::path("logs/server.1.log"));
    CHECK(logging::detail::rotated_path("server", 2) == fs::path("server.2"));
    CHECK(logging::detail::rotated_path("a.b.log", 3) == fs::path("a.b.3.log"));
}

// Messages of 9 characters make lines of 10 bytes; with max_bytes 25 a file holds two lines.
TEST_CASE("rotating file sinks roll over at the size") {
    support::TempDir dir;
    const fs::path path = dir.path() / "server.log";
    {
        logging::RotatingFileSink sink(path, {.max_bytes = 25, .max_files = 2}, support::message_layout());
        for (char c = '1'; c <= '7'; ++c) put(sink, std::string(8, 'L') + c);
    }
    CHECK(support::read_file(path) == "LLLLLLLL7\n");
    CHECK(support::read_file(dir.path() / "server.1.log") == "LLLLLLLL5\nLLLLLLLL6\n");
    CHECK(support::read_file(dir.path() / "server.2.log") == "LLLLLLLL3\nLLLLLLLL4\n");
    CHECK(!fs::exists(dir.path() / "server.3.log"));  // lines 1 and 2 went with the oldest file
}

TEST_CASE("with max_files 0 the file is truncated instead") {
    support::TempDir dir;
    const fs::path path = dir.path() / "server.log";
    {
        logging::RotatingFileSink sink(path, {.max_bytes = 25, .max_files = 0}, support::message_layout());
        for (char c = '1'; c <= '5'; ++c) put(sink, std::string(8, 'L') + c);
    }
    CHECK(support::read_file(path) == "LLLLLLLL5\n");
    CHECK(!fs::exists(dir.path() / "server.1.log"));
}

TEST_CASE("rotating file sinks count an existing file") {
    support::TempDir dir;
    const fs::path path = dir.path() / "server.log";
    {
        logging::FileSink earlier(path, {}, support::message_layout());
        put(earlier, "OLDOLDOL1");
        put(earlier, "OLDOLDOL2");
    }
    {
        logging::RotatingFileSink sink(path, {.max_bytes = 25, .max_files = 1}, support::message_layout());
        put(sink, "NEWNEWNE1");  // 20 + 10 > 25: rotates first
    }
    CHECK(support::read_file(path) == "NEWNEWNE1\n");
    CHECK(support::read_file(dir.path() / "server.1.log") == "OLDOLDOL1\nOLDOLDOL2\n");
}

TEST_CASE("a line longer than max_bytes gets a file of its own") {
    support::TempDir dir;
    const fs::path path = dir.path() / "server.log";
    {
        logging::RotatingFileSink sink(path, {.max_bytes = 5, .max_files = 3}, support::message_layout());
        put(sink, "a long line");
        put(sink, "another long one");
    }
    CHECK(support::read_file(path) == "another long one\n");
    CHECK(support::read_file(dir.path() / "server.1.log") == "a long line\n");
}

TEST_CASE("rotating file sinks need max_bytes above 0") {
    support::TempDir dir;
    CHECK_THROWS_AS(logging::RotatingFileSink(dir.path() / "x.log", {.max_bytes = 0}), logging::Error);
}
