#include <cpm/log/console_sink.hpp>

#include <utility>

#include "terminal.hpp"

namespace cpm::log {

namespace {

std::FILE* stream_of(Console console) noexcept {
    return console == Console::standard_output ? stdout : stderr;
}

bool decide(Color color, std::FILE* file) noexcept {
    switch (color) {
        case Color::always: return true;
        case Color::never: return false;
        case Color::automatic: return detail::terminal::is_terminal(file) && !detail::terminal::no_color();
    }
    return false;
}

// The escape sequence that starts a line's color, or "" for none.
std::string_view color_of(Level level) noexcept {
    switch (level) {
        case Level::trace:
        case Level::debug: return "\x1b[90m";
        case Level::warning: return "\x1b[33m";
        case Level::error: return "\x1b[31m";
        case Level::critical: return "\x1b[1;31m";
        default: return "";
    }
}

void put(std::FILE* file, std::string_view text) noexcept { std::fwrite(text.data(), 1, text.size(), file); }

}  // namespace

ConsoleSink::ConsoleSink(Console console, Color color, std::shared_ptr<const Layout> layout)
    : ConsoleSink(stream_of(console), color, std::move(layout)) {}

ConsoleSink::ConsoleSink(std::FILE* file, Color color, std::shared_ptr<const Layout> layout)
    : TextSink(std::move(layout)), file_(file), colored_(decide(color, file)) {
    if (colored_) detail::terminal::enable_escape_sequences(file_);
}

void ConsoleSink::write_line(std::string_view line, const Record& record) {
    const std::string_view color = colored_ ? color_of(record.level) : std::string_view();
    if (color.empty()) {
        put(file_, line);
    } else {
        // The color ends before the line break, so a terminal doesn't carry it to the next line.
        put(file_, color);
        put(file_, line.substr(0, line.size() - 1));
        put(file_, "\x1b[0m\n");
    }
    std::fflush(file_);
}

void ConsoleSink::flush_lines() { std::fflush(file_); }

}  // namespace cpm::log
