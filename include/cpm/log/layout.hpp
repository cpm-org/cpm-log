// How a record becomes a line of text (docs/spec.md §6).
#pragma once

#include <cpm/log/record.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace cpm::log {

// Turns records into lines. Layouts are immutable, so sinks can share one.
class Layout {
public:
    virtual ~Layout() = default;

    // Appends the record to `out` as one line, without a line break.
    virtual void format(const Record& record, std::string& out) const = 0;

protected:
    Layout() = default;
    Layout(const Layout&) = default;
    Layout& operator=(const Layout&) = default;
};

inline constexpr std::string_view default_pattern = "{time} {level:<8} {logger}: {message}";

namespace detail {

enum class Field { time, utc, level, logger, message, file, path, line, function, thread };

// A piece of a parsed pattern: literal text, or a field with the std::format string that applies its specification
// ("{:<8}"; empty when there is none).
struct PatternSegment {
    std::string text;
    bool is_field = false;
    Field field = Field::message;
    std::string format;
};

}  // namespace detail

// Text with fields in braces: {time} {utc} {level} {logger} {message} {file} {path} {line} {function} {thread}, each
// with an optional std::format specification after a colon ({level:<8}); {{ and }} are braces.
class PatternLayout final : public Layout {
public:
    // Throws cpm::log::Error for a pattern that isn't valid, naming the problem and its position.
    explicit PatternLayout(std::string_view pattern = default_pattern);

    void format(const Record& record, std::string& out) const override;

private:
    std::vector<detail::PatternSegment> segments_;
};

// One JSON object per record: time (UTC), level, logger, message, file, line, function and thread. Always valid JSON:
// bytes that aren't UTF-8 become U+FFFD.
class JsonLayout final : public Layout {
public:
    void format(const Record& record, std::string& out) const override;
};

// The layout sinks use when given none: a shared PatternLayout with default_pattern.
std::shared_ptr<const Layout> default_layout();

}  // namespace cpm::log
