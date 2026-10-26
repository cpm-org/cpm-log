// Loggers (docs/spec.md §4): levels, delivery, the caller's location, laziness, errors and threads.

#include <cpm/log.hpp>
#include <cpm/test.hpp>

#include <format>
#include <memory>
#include <set>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "support/helpers.hpp"

namespace logging = cpm::log;
using logging::Level;
using Lines = std::vector<std::string>;

namespace {

// Counts how often it is formatted.
struct Counted {
    int* calls;
};

// Fails when formatted.
struct Throws {};

class ThrowingSink final : public logging::Sink {
public:
    void write(const logging::Record&) override { throw std::runtime_error("the sink failed"); }
    void flush() override { throw std::runtime_error("the flush failed"); }
};

std::shared_ptr<logging::MemorySink> memory(std::string_view pattern = "{level} {message}") {
    return std::make_shared<logging::MemorySink>(std::make_shared<logging::PatternLayout>(pattern));
}

}  // namespace

template <>
struct std::formatter<Counted> : std::formatter<std::string_view> {
    auto format(const Counted& counted, std::format_context& context) const {
        ++*counted.calls;
        return std::formatter<std::string_view>::format("counted", context);
    }
};

template <>
struct std::formatter<Throws> : std::formatter<std::string_view> {
    auto format(const Throws&, std::format_context& context) const -> std::format_context::iterator {
        static_cast<void>(context);
        throw std::runtime_error("the formatter failed");
    }
};

TEST_CASE("loggers pass records at their level and above to their sinks") {
    auto sink = memory();
    logging::Logger logger("app", {sink});
    CHECK(logger.name() == "app");
    CHECK(logger.level() == Level::info);
    logger.trace("t");
    logger.debug("d {}", 1);
    logger.info("i {} {}", 2, "two");
    logger.warning("w");
    logger.error("e {:>3}", 7);
    logger.critical("c");
    logger.log(Level::warning, "l {}", 3.5);
    CHECK(sink->lines() == (Lines{"info i 2 two", "warning w", "error e   7", "critical c", "warning l 3.5"}));
}

TEST_CASE("levels can change, and off turns a logger off") {
    auto sink = memory("{message}");
    logging::Logger logger("app", {sink}, Level::trace);
    CHECK(logger.enabled(Level::trace));
    logger.trace("1");
    logger.set_level(Level::error);
    CHECK(!logger.enabled(Level::warning));
    CHECK(logger.enabled(Level::critical));
    logger.warning("2");
    logger.error("3");
    logger.set_level(Level::off);
    CHECK(!logger.enabled(Level::critical));
    logger.critical("4");
    logger.set_level(Level::trace);
    logger.log(Level::off, "5");  // logging at off does nothing
    CHECK(!logger.enabled(Level::off));
    CHECK(sink->lines() == (Lines{"1", "3"}));
}

TEST_CASE("records say where the call is, without macros") {
    auto sink = memory("{file}:{line}");
    logging::Logger logger("app", {sink});
    const auto line = std::source_location::current().line(); logger.info("here");
    CHECK(sink->lines() == (Lines{"logger.test.cpp:" + std::to_string(line)}));

    SUBCASE("and which function it is in") {
        auto functions = memory("{function}");
        logging::Logger other("app", {functions});
        other.info("x");
        REQUIRE(functions->lines().size() == 1);
        CHECK(functions->lines()[0] == std::source_location::current().function_name());
    }
}

TEST_CASE("records carry the time, the logger's name and the thread") {
    logging::Record seen;
    std::string name;
    auto sink = std::make_shared<logging::CallbackSink>([&](const logging::Record& r) {
        seen = r;
        name = r.logger;
    });
    logging::Logger logger("worker", {sink});
    const auto before = std::chrono::system_clock::now();
    logger.info("tick");
    const auto after = std::chrono::system_clock::now();
    CHECK(name == "worker");
    CHECK(seen.level == Level::info);
    CHECK(seen.time >= before);
    CHECK(seen.time <= after);
    CHECK(seen.thread == logging::current_thread_number());
}

TEST_CASE("a message is formatted only when its level is enabled") {
    int calls = 0;
    auto sink = memory("{message}");
    logging::Logger logger("app", {sink});
    logger.debug("{}", Counted{&calls});
    CHECK(calls == 0);
    logger.info("{}", Counted{&calls});
    CHECK(calls == 1);
    CHECK(sink->lines() == (Lines{"counted"}));
}

TEST_CASE("logging never throws: failures go to the error handler") {
    std::vector<std::string> errors;
    auto good = memory("{message}");
    logging::Logger logger("app", {std::make_shared<ThrowingSink>(), good});
    logger.set_error_handler([&](const std::exception& e) { errors.emplace_back(e.what()); });

    logger.info("still {}", "delivered");
    CHECK(good->lines() == (Lines{"still delivered"}));  // the other sink still gets the record
    CHECK(errors == (Lines{"the sink failed"}));

    logger.info("{}", Throws{});  // formatting fails: the record is dropped
    CHECK(good->lines().size() == 1);
    CHECK(errors.back() == "the formatter failed");

    logger.flush();
    CHECK(errors.back() == "the flush failed");

    SUBCASE("a file sink that fails reaches the handler too") {
        std::ostringstream failed;
        failed.setstate(std::ios::badbit);
        logger.set_sinks({std::make_shared<logging::StreamSink>(failed)});
        logger.error("x");
        CHECK(errors.back() == "writing a log line to the stream failed");
    }
}

TEST_CASE("sinks can be replaced, and shared between loggers") {
    auto first = memory("{message}");
    auto second = memory("{message}");
    logging::Logger a("a", {first});
    logging::Logger b("b", {first});
    a.info("a1");
    b.info("b1");
    a.set_sinks({second});
    a.info("a2");
    CHECK(first->lines() == (Lines{"a1", "b1"}));
    CHECK(second->lines() == (Lines{"a2"}));
    CHECK(a.sinks().size() == 1);
    CHECK(a.sinks()[0] == second);
}

TEST_CASE("threads get small numbers of their own") {
    const std::uint64_t mine = logging::current_thread_number();
    CHECK(mine == logging::current_thread_number());
    std::uint64_t theirs = 0;
    std::thread([&] { theirs = logging::current_thread_number(); }).join();
    CHECK(theirs != 0);
    CHECK(theirs != mine);
}

TEST_CASE("many threads log whole lines") {
    auto sink = memory("{message}");
    logging::Logger logger("app", {sink}, Level::trace);
    constexpr int threads = 8;
    constexpr int each = 1000;
    std::vector<std::thread> workers;
    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&, t] {
            for (int i = 0; i < each; ++i) logger.info("thread {} message {} {}", t, i, std::string(40, 'x'));
            if (t == 0) logger.set_level(Level::trace);  // changing settings while others log is safe
        });
    }
    for (std::thread& worker : workers) worker.join();
    const Lines lines = sink->lines();
    CHECK(lines.size() == static_cast<std::size_t>(threads * each));
    const std::set<std::string> distinct(lines.begin(), lines.end());
    CHECK(distinct.size() == lines.size());
    for (const std::string& line : lines) {
        if (!line.starts_with("thread ") || !line.ends_with(std::string(40, 'x'))) {
            FAIL_CHECK("a broken line: {}", line);
            break;
        }
    }
}
