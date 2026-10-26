#include <cpm/log/logger.hpp>
#include <cpm/log/record.hpp>

#include <chrono>
#include <cstdio>
#include <stdexcept>
#include <utility>

namespace cpm::log {

Logger::Logger(std::string name, Sinks sinks, Level level)
    : name_(std::move(name)), level_(level), sinks_(std::make_shared<const Sinks>(std::move(sinks))) {}

Logger::~Logger() = default;

Logger::Sinks Logger::sinks() const { return *current_sinks(); }

void Logger::set_sinks(Sinks sinks) {
    auto replacement = std::make_shared<const Sinks>(std::move(sinks));
    const std::lock_guard lock(mutex_);
    sinks_ = std::move(replacement);
}

void Logger::set_error_handler(ErrorHandler handler) {
    auto replacement = handler ? std::make_shared<const ErrorHandler>(std::move(handler)) : nullptr;
    const std::lock_guard lock(mutex_);
    handler_ = std::move(replacement);
}

std::shared_ptr<const Logger::Sinks> Logger::current_sinks() const {
    const std::lock_guard lock(mutex_);
    return sinks_;
}

void Logger::deliver(Level level, std::string_view message, const std::source_location& where) {
    const Record record{std::chrono::system_clock::now(), level, name_, message, where, current_thread_number()};
    const std::shared_ptr<const Sinks> sinks = current_sinks();
    for (const std::shared_ptr<Sink>& sink : *sinks) {
        try {
            sink->write(record);
        } catch (const std::exception& failure) {
            report(failure);  // the other sinks still get the record
        } catch (...) {
            report_unknown();
        }
    }
}

void Logger::flush() noexcept {
    try {
        const std::shared_ptr<const Sinks> sinks = current_sinks();  // held, so set_sinks can't free them meanwhile
        for (const std::shared_ptr<Sink>& sink : *sinks) {
            try {
                sink->flush();
            } catch (const std::exception& failure) {
                report(failure);
            } catch (...) {
                report_unknown();
            }
        }
    } catch (const std::exception& failure) {
        report(failure);
    }
}

void Logger::report(const std::exception& failure) noexcept {
    std::shared_ptr<const ErrorHandler> handler;
    {
        const std::lock_guard lock(mutex_);
        handler = handler_;
    }
    if (handler) {
        (*handler)(failure);
        return;
    }
    if (!reported_.exchange(true)) {
        std::fprintf(stderr, "cpm-log: logger \"%s\": %s\n", name_.c_str(), failure.what());
        std::fflush(stderr);
    }
}

void Logger::report_unknown() noexcept { report(std::runtime_error("an exception that isn't a std::exception")); }

}  // namespace cpm::log
