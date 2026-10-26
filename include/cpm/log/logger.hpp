// Loggers (docs/spec.md §4).
#pragma once

#include <cpm/log/format.hpp>
#include <cpm/log/level.hpp>
#include <cpm/log/sink.hpp>

#include <atomic>
#include <exception>
#include <format>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cpm::log {

// A name, a level and sinks. Every member is safe to call from many threads at once, and logging never throws: when
// formatting a message or a sink throws, the exception goes to the error handler.
class Logger {
public:
    using Sinks = std::vector<std::shared_ptr<Sink>>;
    using ErrorHandler = std::function<void(const std::exception&)>;

    explicit Logger(std::string name, Sinks sinks = {}, Level level = Level::info);
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    ~Logger();

    const std::string& name() const noexcept { return name_; }

    Level level() const noexcept { return level_.load(std::memory_order_relaxed); }
    void set_level(Level level) noexcept { level_.store(level, std::memory_order_relaxed); }

    // Whether a record at `level` passes the logger's level. Guard expensive arguments with it.
    bool enabled(Level level) const noexcept {
        const Level threshold = this->level();
        return level != Level::off && threshold != Level::off && level >= threshold;
    }

    Sinks sinks() const;
    // Takes effect for later records; records being written finish with the old sinks.
    void set_sinks(Sinks sinks);

    // Receives the exceptions from formatting and from sinks. It must not throw. nullptr restores the default, which
    // writes the first one to standard error and stays quiet afterwards.
    void set_error_handler(ErrorHandler handler);

    // Formats the message only when `level` is enabled, then passes the record to every sink.
    template <class... Args>
    void log(Level level, FormatFor<Args...> format, Args&&... args) noexcept {
        if (!enabled(level)) return;
        try {
            std::string message;
            std::format_to(std::back_inserter(message), format.text(), std::forward<Args>(args)...);
            deliver(level, message, format.where());
        } catch (const std::exception& failure) {
            report(failure);
        } catch (...) {
            report_unknown();
        }
    }

    template <class... Args>
    void trace(FormatFor<Args...> format, Args&&... args) noexcept {
        log(Level::trace, format, std::forward<Args>(args)...);
    }
    template <class... Args>
    void debug(FormatFor<Args...> format, Args&&... args) noexcept {
        log(Level::debug, format, std::forward<Args>(args)...);
    }
    template <class... Args>
    void info(FormatFor<Args...> format, Args&&... args) noexcept {
        log(Level::info, format, std::forward<Args>(args)...);
    }
    template <class... Args>
    void warning(FormatFor<Args...> format, Args&&... args) noexcept {
        log(Level::warning, format, std::forward<Args>(args)...);
    }
    template <class... Args>
    void error(FormatFor<Args...> format, Args&&... args) noexcept {
        log(Level::error, format, std::forward<Args>(args)...);
    }
    template <class... Args>
    void critical(FormatFor<Args...> format, Args&&... args) noexcept {
        log(Level::critical, format, std::forward<Args>(args)...);
    }

    // Flushes every sink.
    void flush() noexcept;

private:
    void deliver(Level level, std::string_view message, const std::source_location& where);
    void report(const std::exception& failure) noexcept;
    void report_unknown() noexcept;
    std::shared_ptr<const Sinks> current_sinks() const;

    const std::string name_;
    std::atomic<Level> level_;
    mutable std::mutex mutex_;  // guards sinks_ and handler_
    std::shared_ptr<const Sinks> sinks_;
    std::shared_ptr<const ErrorHandler> handler_;
    std::atomic<bool> reported_{false};  // the default handler has spoken
};

}  // namespace cpm::log
