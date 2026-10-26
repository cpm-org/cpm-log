// What the console can show (docs/spec.md §5.1). The one place that asks the platform about terminals.
#pragma once

#include <cstdio>

namespace cpm::log::detail::terminal {

// Whether `file` is a terminal (a console window on Windows).
bool is_terminal(std::FILE* file) noexcept;

// Whether the NO_COLOR environment variable asks for no colors: set and not empty (no-color.org).
bool no_color() noexcept;

// Makes the console behind `file` interpret escape sequences: needed on Windows, nothing elsewhere.
void enable_escape_sequences(std::FILE* file) noexcept;

}  // namespace cpm::log::detail::terminal
