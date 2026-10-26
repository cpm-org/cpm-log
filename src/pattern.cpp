#include "pattern.hpp"

#include <cpm/log/error.hpp>

#include <array>
#include <cstdint>
#include <format>
#include <string>
#include <utility>

namespace cpm::log::detail {

namespace {

struct FieldInfo {
    std::string_view name;
    Field field;
    bool number;      // formatted as an integer, else as text
    bool takes_spec;  // times take no specification
};

constexpr std::array<FieldInfo, 10> field_infos = {{
    {"time", Field::time, false, false},
    {"utc", Field::utc, false, false},
    {"level", Field::level, false, true},
    {"logger", Field::logger, false, true},
    {"message", Field::message, false, true},
    {"file", Field::file, false, true},
    {"path", Field::path, false, true},
    {"line", Field::line, true, true},
    {"function", Field::function, false, true},
    {"thread", Field::thread, true, true},
}};

const FieldInfo* find_field(std::string_view name) noexcept {
    for (const FieldInfo& info : field_infos) {
        if (info.name == name) return &info;
    }
    return nullptr;
}

[[noreturn]] void invalid(std::string_view pattern, std::size_t position, std::string_view problem) {
    throw Error(std::format("the log pattern \"{}\" isn't valid: {} at position {}", pattern, problem, position + 1));
}

// Whether `format` ("{:<8}") can format the field's values.
bool accepts(const FieldInfo& info, const std::string& format) {
    try {
        if (info.number) {
            const std::uint64_t sample = 1;
            static_cast<void>(std::vformat(format, std::make_format_args(sample)));
        } else {
            const std::string_view sample = "text";
            static_cast<void>(std::vformat(format, std::make_format_args(sample)));
        }
        return true;
    } catch (const std::format_error&) {
        return false;
    }
}

}  // namespace

std::vector<PatternSegment> parse_pattern(std::string_view pattern) {
    std::vector<PatternSegment> segments;
    std::string text;
    auto end_text = [&] {
        if (text.empty()) return;
        segments.push_back({std::move(text), false, Field::message, {}});
        text.clear();
    };
    for (std::size_t i = 0; i < pattern.size(); ++i) {
        const char c = pattern[i];
        const bool doubled = i + 1 < pattern.size() && pattern[i + 1] == c;
        if (c == '}') {
            if (!doubled) invalid(pattern, i, "a '}' that closes nothing (write }} for a brace)");
            text += '}';
            ++i;
            continue;
        }
        if (c != '{') {
            text += c;
            continue;
        }
        if (doubled) {
            text += '{';
            ++i;
            continue;
        }
        const std::size_t close = pattern.find('}', i + 1);
        if (close == std::string_view::npos) invalid(pattern, i, "a '{' that isn't closed");
        const std::string_view inside = pattern.substr(i + 1, close - i - 1);
        const std::size_t colon = inside.find(':');
        const std::string_view name = inside.substr(0, colon);
        const FieldInfo* info = find_field(name);
        if (info == nullptr) invalid(pattern, i, std::format("an unknown field {{{}}}", name));
        PatternSegment segment{{}, true, info->field, {}};
        if (colon != std::string_view::npos) {
            const std::string_view spec = inside.substr(colon + 1);
            if (!info->takes_spec) invalid(pattern, i, std::format("{{{}}} takes no format specification", name));
            segment.format = std::format("{{:{}}}", spec);
            if (!accepts(*info, segment.format)) {
                invalid(pattern, i, std::format("{{{}}} can't take the format specification \"{}\"", name, spec));
            }
        }
        end_text();
        segments.push_back(std::move(segment));
        i = close;
    }
    end_text();
    return segments;
}

}  // namespace cpm::log::detail
