// Passing records to a function (docs/spec.md §5.4).
#pragma once

#include <cpm/log/sink.hpp>

#include <functional>
#include <mutex>

namespace cpm::log {

// Passes records at `level` and above to the callback, one call at a time: the way to forward records to another
// system. Exceptions from the callback reach the logger's error handler.
class CallbackSink final : public Sink {
public:
    explicit CallbackSink(std::function<void(const Record&)> callback, Level level = Level::trace);

    void write(const Record& record) override;
    void flush() override {}

private:
    std::function<void(const Record&)> callback_;
    Level level_;
    std::mutex mutex_;
};

}  // namespace cpm::log
