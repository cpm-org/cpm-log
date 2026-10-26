#include "terminal.hpp"

#include <cstdlib>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace cpm::log::detail::terminal {

bool is_terminal(std::FILE* file) noexcept {
#if defined(_WIN32)
    return _isatty(_fileno(file)) != 0;
#else
    return isatty(fileno(file)) != 0;
#endif
}

bool no_color() noexcept {
    const char* value = std::getenv("NO_COLOR");
    return value != nullptr && *value != '\0';
}

void enable_escape_sequences([[maybe_unused]] std::FILE* file) noexcept {
#if defined(_WIN32)
    const HANDLE console = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file)));
    DWORD mode = 0;
    if (console != INVALID_HANDLE_VALUE && GetConsoleMode(console, &mode)) {
        SetConsoleMode(console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif
}

}  // namespace cpm::log::detail::terminal
