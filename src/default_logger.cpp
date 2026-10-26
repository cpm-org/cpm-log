#include <cpm/log/console_sink.hpp>
#include <cpm/log/default_logger.hpp>

#include <memory>

namespace cpm::log {

Logger& default_logger() {
    // Never destroyed: code running while statics are destroyed may still log.
    static Logger* const logger = new Logger("main", {std::make_shared<ConsoleSink>()}, Level::info);
    return *logger;
}

}  // namespace cpm::log
