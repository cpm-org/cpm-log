// Logging to the console, colored by level (docs/spec.md §5.1).
#pragma once

#include <cpm/log/sink.hpp>

#include <cstdio>
#include <memory>

namespace cpm::log {

enum class Console { standard_error, standard_output };

// automatic: color when the stream is a terminal and NO_COLOR isn't set.
enum class Color { automatic, always, never };

// Writes lines to standard error or output (or any C stream), flushed after every line. Colored lines are dim for
// trace and debug, yellow for warning, red for error and bold red for critical; info isn't colored.
class ConsoleSink final : public TextSink {
public:
    explicit ConsoleSink(Console console = Console::standard_error, Color color = Color::automatic,
                         std::shared_ptr<const Layout> layout = nullptr);
    ConsoleSink(std::FILE* file, Color color, std::shared_ptr<const Layout> layout = nullptr);

    // Whether lines are colored, as decided when the sink was made.
    bool colored() const noexcept { return colored_; }

protected:
    void write_line(std::string_view line, const Record& record) override;
    void flush_lines() override;

private:
    std::FILE* file_;
    bool colored_;
};

}  // namespace cpm::log
