#include "utf8.hpp"

namespace cpm::log::detail::utf8 {

std::size_t sequence_length(std::string_view text, std::size_t pos) noexcept {
    auto byte = [&](std::size_t i) { return static_cast<unsigned char>(text[pos + i]); };
    auto continues = [&](std::size_t i, unsigned char low = 0x80, unsigned char high = 0xBF) {
        return byte(i) >= low && byte(i) <= high;
    };
    const std::size_t left = text.size() - pos;
    const unsigned char lead = byte(0);
    if (lead < 0x80) return 1;
    if (lead >= 0xC2 && lead <= 0xDF) return left >= 2 && continues(1) ? 2 : 0;
    if (lead >= 0xE0 && lead <= 0xEF) {
        // E0 would be overlong below A0; ED would encode a surrogate from A0.
        const unsigned char low = lead == 0xE0 ? 0xA0 : 0x80;
        const unsigned char high = lead == 0xED ? 0x9F : 0xBF;
        return left >= 3 && continues(1, low, high) && continues(2) ? 3 : 0;
    }
    if (lead >= 0xF0 && lead <= 0xF4) {
        // F0 would be overlong below 90; F4 would pass U+10FFFF from 90.
        const unsigned char low = lead == 0xF0 ? 0x90 : 0x80;
        const unsigned char high = lead == 0xF4 ? 0x8F : 0xBF;
        return left >= 4 && continues(1, low, high) && continues(2) && continues(3) ? 4 : 0;
    }
    return 0;
}

}  // namespace cpm::log::detail::utf8
