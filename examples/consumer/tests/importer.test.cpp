#include <consumer/importer.hpp>

#include <cpm/test.hpp>

#include <memory>

namespace logging = cpm::log;

TEST_CASE("the importer logs what it does") {
    auto lines = std::make_shared<logging::MemorySink>(std::make_shared<logging::PatternLayout>("{level} {message}"));
    logging::Logger logger("import", {lines}, logging::Level::debug);

    CHECK(import_rows({"port=80", "oops", "host=example.com"}, logger) == 2);
    CHECK(lines->lines() == (std::vector<std::string>{
                                "debug row 1: port is 80",
                                "warning row 2 skipped: expected name=value, found \"oops\"",
                                "debug row 3: host is example.com",
                                "info imported 2 of 3 rows",
                            }));

    SUBCASE("and less at a higher level") {
        lines->clear();
        logger.set_level(logging::Level::warning);
        import_rows({"=x"}, logger);
        CHECK(lines->lines() == (std::vector<std::string>{"warning row 1 skipped: expected name=value, found \"=x\""}));
    }
}
