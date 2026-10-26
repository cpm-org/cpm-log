// Sinks (docs/spec.md §5.1, §5.4): levels, memory, streams, the console's colors, and callbacks.

#include <cpm/log.hpp>
#include <cpm/test.hpp>

#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

#include "support/helpers.hpp"

namespace logging = cpm::log;
using logging::Level;
using Lines = std::vector<std::string>;

TEST_CASE("text sinks keep a level of their own") {
    logging::MemorySink sink(support::message_layout());
    CHECK(sink.level() == Level::trace);
    sink.write(support::record(Level::trace, "a"));
    sink.set_level(Level::warning);
    sink.write(support::record(Level::info, "b"));
    sink.write(support::record(Level::warning, "c"));
    sink.write(support::record(Level::critical, "d"));
    sink.set_level(Level::off);
    sink.write(support::record(Level::critical, "e"));
    CHECK(sink.lines() == (Lines{"a", "c", "d"}));
    sink.clear();
    CHECK(sink.lines().empty());
}

TEST_CASE("text sinks use the default layout unless given one") {
    logging::MemorySink sink;
    sink.write(support::record(Level::error, "boom"));
    REQUIRE(sink.lines().size() == 1);
    CHECK(sink.lines()[0].ends_with(" error    test: boom"));
}

TEST_CASE("stream sinks write lines to streams") {
    std::ostringstream out;
    logging::StreamSink sink(out, support::message_layout());
    sink.write(support::record(Level::info, "one"));
    sink.write(support::record(Level::info, "two"));
    sink.flush();
    CHECK(out.str() == "one\ntwo\n");

    SUBCASE("and say when a stream fails") {
        std::ostringstream failed;
        failed.setstate(std::ios::badbit);
        logging::StreamSink bad(failed, support::message_layout());
        CHECK_THROWS_AS(bad.write(support::record(Level::info, "x")), logging::Error);
    }
}

TEST_CASE("console sinks color whole lines by level") {
    support::TempDir dir;
    const auto path = dir.path() / "console.txt";
    {
        std::FILE* file = std::fopen(path.string().c_str(), "wb");
        REQUIRE(file != nullptr);
        logging::ConsoleSink sink(file, logging::Color::always, support::message_layout());
        CHECK(sink.colored());
        for (Level level : {Level::trace, Level::debug, Level::info, Level::warning, Level::error, Level::critical})
            sink.write(support::record(level, logging::to_string(level)));
        std::fclose(file);
    }
    CHECK(support::read_file(path) ==
          "\x1b[90mtrace\x1b[0m\n"
          "\x1b[90mdebug\x1b[0m\n"
          "info\n"
          "\x1b[33mwarning\x1b[0m\n"
          "\x1b[31merror\x1b[0m\n"
          "\x1b[1;31mcritical\x1b[0m\n");
}

TEST_CASE("console sinks leave colors out when asked, or when the stream isn't a terminal") {
    support::TempDir dir;
    const auto path = dir.path() / "console.txt";
    std::FILE* file = std::fopen(path.string().c_str(), "wb");
    REQUIRE(file != nullptr);
    {
        logging::ConsoleSink never(file, logging::Color::never, support::message_layout());
        CHECK(!never.colored());
        never.write(support::record(Level::error, "plain"));
        const logging::ConsoleSink automatic(file, logging::Color::automatic);
        CHECK(!automatic.colored());  // a file isn't a terminal
    }
    std::fclose(file);
    CHECK(support::read_file(path) == "plain\n");
}

TEST_CASE("callback sinks pass records on") {
    Lines seen;
    logging::CallbackSink sink(
        [&](const logging::Record& r) { seen.push_back(std::string(logging::to_string(r.level)) + ":" + std::string(r.message)); },
        Level::info);
    sink.write(support::record(Level::debug, "dropped"));
    sink.write(support::record(Level::info, "kept"));
    sink.write(support::record(Level::error, "too"));
    sink.flush();
    CHECK(seen == (Lines{"info:kept", "error:too"}));
}
