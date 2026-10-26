// Levels (docs/spec.md §3).
#pragma once

#include <optional>
#include <string_view>

namespace cpm::log {

// In order: a logger or sink at level L takes records at L and above; `off` takes none.
enum class Level { trace, debug, info, warning, error, critical, off };

// The lowercase name: "trace", ..., "critical", "off".
std::string_view to_string(Level level) noexcept;

// The level named by `text`, in any case; "warn" is warning. nullopt for anything else.
std::optional<Level> parse_level(std::string_view text) noexcept;

}  // namespace cpm::log
