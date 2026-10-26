// The default logger and the free functions (docs/spec.md §4.5).

#include <cpm/log.hpp>
#include <cpm/test.hpp>

#include <memory>
#include <string>
#include <vector>

namespace logging = cpm::log;
using logging::Level;

TEST_CASE("the default logger is main, at info, on the console") {
    logging::Logger& logger = logging::default_logger();
    CHECK(&logger == &logging::default_logger());
    CHECK(logger.name() == "main");
    CHECK(logger.level() == Level::info);
    const auto sinks = logger.sinks();
    REQUIRE(sinks.size() == 1);
    CHECK(dynamic_cast<logging::ConsoleSink*>(sinks[0].get()) != nullptr);
}

TEST_CASE("the free functions log to the default logger") {
    logging::Logger& logger = logging::default_logger();
    const auto original = logger.sinks();
    auto sink = std::make_shared<logging::MemorySink>(std::make_shared<logging::PatternLayout>("{logger} {level} {message}"));
    logger.set_sinks({sink});
    logger.set_level(Level::trace);

    logging::trace("t");
    logging::debug("d");
    logging::info("i {}", 1);
    logging::warning("w");
    logging::error("e");
    logging::critical("c");
    logging::log(Level::info, "l {}", "x");

    logger.set_sinks(original);
    logger.set_level(Level::info);
    CHECK(sink->lines() == (std::vector<std::string>{"main trace t", "main debug d", "main info i 1", "main warning w",
                                                     "main error e", "main critical c", "main info l x"}));
}
