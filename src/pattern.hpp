// Parsing patterns (docs/spec.md §6.1).
#pragma once

#include <cpm/log/layout.hpp>

#include <string_view>
#include <vector>

namespace cpm::log::detail {

// The pattern as literal text and fields, adjacent text merged. Throws cpm::log::Error for an unknown field, a brace
// that isn't closed or matched, or a specification its field can't take.
std::vector<PatternSegment> parse_pattern(std::string_view pattern);

}  // namespace cpm::log::detail
