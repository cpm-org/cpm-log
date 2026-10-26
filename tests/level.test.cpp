// Levels (docs/spec.md §3).

#include <cpm/log.hpp>
#include <cpm/test.hpp>

using cpm::log::Level;

TEST_CASE("levels have lowercase names") {
    CHECK(cpm::log::to_string(Level::trace) == "trace");
    CHECK(cpm::log::to_string(Level::debug) == "debug");
    CHECK(cpm::log::to_string(Level::info) == "info");
    CHECK(cpm::log::to_string(Level::warning) == "warning");
    CHECK(cpm::log::to_string(Level::error) == "error");
    CHECK(cpm::log::to_string(Level::critical) == "critical");
    CHECK(cpm::log::to_string(Level::off) == "off");
}

TEST_CASE("levels are ordered") {
    CHECK(Level::trace < Level::debug);
    CHECK(Level::debug < Level::info);
    CHECK(Level::info < Level::warning);
    CHECK(Level::warning < Level::error);
    CHECK(Level::error < Level::critical);
    CHECK(Level::critical < Level::off);
}

TEST_CASE("parse_level reads names in any case") {
    CHECK(cpm::log::parse_level("debug") == Level::debug);
    CHECK(cpm::log::parse_level("WARNING") == Level::warning);
    CHECK(cpm::log::parse_level("Warn") == Level::warning);
    CHECK(cpm::log::parse_level("Off") == Level::off);
    CHECK(cpm::log::parse_level("critical") == Level::critical);
    CHECK(!cpm::log::parse_level("verbose").has_value());
    CHECK(!cpm::log::parse_level("").has_value());
    CHECK(!cpm::log::parse_level("info ").has_value());
}
