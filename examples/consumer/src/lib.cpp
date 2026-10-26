#include <consumer/importer.hpp>

int import_rows(const std::vector<std::string>& rows, cpm::log::Logger& logger) {
    int imported = 0;
    for (std::size_t i = 0; i < rows.size(); ++i) {
        const std::string& row = rows[i];
        const std::size_t equals = row.find('=');
        if (equals == std::string::npos || equals == 0) {
            logger.warning("row {} skipped: expected name=value, found \"{}\"", i + 1, row);
            continue;
        }
        logger.debug("row {}: {} is {}", i + 1, row.substr(0, equals), row.substr(equals + 1));
        ++imported;
    }
    logger.info("imported {} of {} rows", imported, rows.size());
    return imported;
}
