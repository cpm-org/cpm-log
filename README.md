# cpm-log

Logging for C++20, made for [CPM](https://github.com/meirborowski/CPM). Messages are formatted with `std::format`, and the format string is checked at compile time. Each record knows the file and line it came from, without macros. Records go to the console, files, rotating files, or anywhere else, as text or JSON lines.

```sh
cpm install cpm-log
```

```cpp
#include <cpm/log.hpp>

int main() {
    cpm::log::info("listening on {}:{}", "0.0.0.0", 8080);
    cpm::log::warning("cache is {}% full", 91);
}
```

```
2026-10-26 14:03:07.123 info     main: listening on 0.0.0.0:8080
2026-10-26 14:03:07.124 warning  main: cache is 91% full
```

The [specification](docs/spec.md) has every rule in detail.

## Loggers

A logger has a name, a level and sinks:

```cpp
namespace logging = cpm::log;

auto console = std::make_shared<logging::ConsoleSink>();
auto file = std::make_shared<logging::FileSink>("logs/net.log");
logging::Logger net("net", {console, file}, logging::Level::debug);

net.debug("connecting to {}", host);
net.error("lost {} connections", lost);
net.log(logging::Level::warning, "retrying in {} s", delay);
```

- The levels are `trace`, `debug`, `info`, `warning`, `error` and `critical`. A logger takes records at its level and above (default `info`), and `off` turns it off.
- `set_level` and `set_sinks` change a logger while it is in use, from any thread.
- Sinks can be shared between loggers.
- `parse_level("debug")` reads a level from an environment variable or a configuration.

`cpm::log::info(...)` and the other free functions log to the **default logger**:
- it is named `main` and is at level `info`;
- it writes to standard error, in color when that's a terminal;
- configure it through `cpm::log::default_logger()`.

Prefer `cpm::log::...` or another alias such as `logging` over `namespace log = cpm::log;`. `log` at global scope clashes with the C library's `log()`.

**Messages** are formatted only when the level lets them through. Their arguments are still evaluated, as for any function call, so guard expensive ones:

```cpp
if (net.enabled(logging::Level::debug)) net.debug("state: {}", expensive_dump());
```

**Logging never throws.** When formatting a message or a sink fails, the exception goes to the logger's error handler, and the other sinks still get the record. The default handler prints the first failure to standard error and stays quiet after that, so a full disk doesn't flood the console. Set your own with `set_error_handler`.

## Sinks

| Sink | Writes to |
|---|---|
| `ConsoleSink(Console::standard_error, Color::automatic)` | standard error or output, colored by level when it's a terminal and `NO_COLOR` isn't set |
| `FileSink(path, {.append = true, .flush_level = Level::warning})` | a file, creating its directories |
| `RotatingFileSink(path, {.max_bytes = 10 MiB, .max_files = 5})` | a file that rolls over: `app.log`, `app.1.log`, `app.2.log`, … |
| `StreamSink(std::ostream&)` | any stream |
| `MemorySink()` | a list of lines, for tests |
| `CallbackSink(function, level)` | a function you give it, one record at a time |

- Every sink but `CallbackSink` has its own level, so one logger can send everything to a file and only warnings to the console: `console->set_level(logging::Level::warning)`.
- Lines from different threads never interleave.
- A sink of your own implements `Sink` (`write(const Record&)` and `flush()`). To get the level, the layout and the locking for free, derive it from `TextSink` instead.

## Layouts

Text sinks take a layout as their last argument. A `PatternLayout` is text with fields in braces:

```cpp
auto layout = std::make_shared<logging::PatternLayout>("{utc} [{level:^8}] {file}:{line} {message}");
auto file = std::make_shared<logging::FileSink>("app.log", logging::FileOptions{}, layout);
```

| Field | |
|---|---|
| `{time}` | local time, `2026-10-26 14:03:07.123` |
| `{utc}` | UTC, `2026-10-26T12:03:07.123Z` |
| `{level}`, `{logger}`, `{message}` | |
| `{file}`, `{path}`, `{line}`, `{function}` | where the call is: file name, full path, line, function |
| `{thread}` | a small number per thread: 1, 2, 3, … |

- Fields take `std::format` specifications, such as `{level:<8}` or `{line:>4}`.
- `{{` and `}}` are literal braces.
- The default is `{time} {level:<8} {logger}: {message}`.
- A pattern that isn't valid throws `cpm::log::Error` when the layout is made, saying where the problem is.

`JsonLayout` writes one JSON object per line, ready for log collectors:

```json
{"time":"2026-10-26T12:03:07.123Z","level":"warning","logger":"net","message":"slow response","file":"src/net.cpp","line":88,"function":"void poll()","thread":3}
```

Every line is valid JSON, even when a message holds bytes that aren't UTF-8.

## Platforms

cpm-log is tested on Linux (GCC and Clang, with AddressSanitizer, UBSan and ThreadSanitizer), macOS (Apple Clang) and Windows (MinGW GCC). It has no dependencies.
