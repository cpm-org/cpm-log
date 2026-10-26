#include <cpm/log/callback_sink.hpp>

#include <utility>

namespace cpm::log {

CallbackSink::CallbackSink(std::function<void(const Record&)> callback, Level level)
    : callback_(std::move(callback)), level_(level) {}

void CallbackSink::write(const Record& record) {
    if (level_ == Level::off || record.level < level_) return;
    const std::lock_guard lock(mutex_);
    callback_(record);
}

}  // namespace cpm::log
