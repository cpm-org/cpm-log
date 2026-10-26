// UTF-8 checking (RFC 3629), for the JSON layout.
#pragma once

#include <cstddef>
#include <string_view>

namespace cpm::log::detail::utf8 {

// The length (1 to 4) of the well-formed sequence that starts at text[pos], or 0 when the bytes there aren't one: a
// stray continuation byte, an overlong form, an encoded surrogate, a code point above U+10FFFF, or a cut-short sequence.
std::size_t sequence_length(std::string_view text, std::size_t pos) noexcept;

}  // namespace cpm::log::detail::utf8
