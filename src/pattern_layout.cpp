#include <cpm/log/layout.hpp>

#include <cstdint>
#include <format>
#include <iterator>

#include "pattern.hpp"
#include "time_text.hpp"

namespace cpm::log {

namespace {

// The source file's name without its directories.
std::string_view file_name(std::string_view path) noexcept {
    const std::size_t slash = path.find_last_of("/\\");
    return slash == std::string_view::npos ? path : path.substr(slash + 1);
}

void put_text(std::string& out, const detail::PatternSegment& segment, std::string_view text) {
    if (segment.format.empty()) {
        out += text;
    } else {
        std::vformat_to(std::back_inserter(out), segment.format, std::make_format_args(text));
    }
}

void put_number(std::string& out, const detail::PatternSegment& segment, std::uint64_t number) {
    std::vformat_to(std::back_inserter(out), segment.format.empty() ? "{}" : segment.format,
                    std::make_format_args(number));
}

}  // namespace

PatternLayout::PatternLayout(std::string_view pattern) : segments_(detail::parse_pattern(pattern)) {}

void PatternLayout::format(const Record& record, std::string& out) const {
    using detail::Field;
    for (const detail::PatternSegment& segment : segments_) {
        if (!segment.is_field) {
            out += segment.text;
            continue;
        }
        switch (segment.field) {
            case Field::time: detail::append_local_time(out, record.time); break;
            case Field::utc: detail::append_utc_time(out, record.time); break;
            case Field::level: put_text(out, segment, to_string(record.level)); break;
            case Field::logger: put_text(out, segment, record.logger); break;
            case Field::message: put_text(out, segment, record.message); break;
            case Field::file: put_text(out, segment, file_name(record.where.file_name())); break;
            case Field::path: put_text(out, segment, record.where.file_name()); break;
            case Field::line: put_number(out, segment, record.where.line()); break;
            case Field::function: put_text(out, segment, record.where.function_name()); break;
            case Field::thread: put_number(out, segment, record.thread); break;
        }
    }
}

std::shared_ptr<const Layout> default_layout() {
    static const std::shared_ptr<const Layout> layout = std::make_shared<PatternLayout>();
    return layout;
}

}  // namespace cpm::log
