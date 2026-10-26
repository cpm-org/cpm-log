// The format string of a log call, checked at compile time, with where the call is (docs/spec.md §4.2).
#pragma once

#include <concepts>
#include <format>
#include <source_location>
#include <string_view>
#include <type_traits>

namespace cpm::log {

// Built implicitly from the string literal at a log call: the consteval constructor checks the format string against
// the arguments, as std::format_string does, and its default argument captures the caller's source location.
template <class... Args>
class Format {
public:
    template <class S>
        requires std::convertible_to<const S&, std::string_view>
    consteval Format(const S& text, std::source_location where = std::source_location::current())
        : text_(text), where_(where) {}

    std::format_string<Args...> text() const noexcept { return text_; }
    const std::source_location& where() const noexcept { return where_; }

private:
    std::format_string<Args...> text_;
    std::source_location where_;
};

// The parameter type of log calls: type_identity keeps the arguments, not the format string, deciding Args.
template <class... Args>
using FormatFor = Format<std::type_identity_t<Args>...>;

}  // namespace cpm::log
