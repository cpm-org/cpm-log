# cpm-log 0.1.0: specification

cpm-log is logging for C++20: leveled messages formatted with `std::format`, sent to the console, files and other sinks, in text or JSON lines. It is a [cpm-org](https://github.com/cpm-org) library: original code with its own API, built and tested with [CPM](https://github.com/meirborowski/CPM). This document is what the code is written against. The README will be the user's guide.

## 1. Scope

0.1.0 covers:

- **Levels:** `trace`, `debug`, `info`, `warning`, `error` and `critical`.
- **Loggers:** named, each with a level and a list of sinks, plus a default logger behind free functions.
- **Messages** formatted with `std::format`, with the format string checked at compile time. The source location is captured without macros, and a message is formatted only when some sink will take it.
- **Sinks:** the console (with colors), streams, files, size-rotated files, memory and callbacks. Sinks are safe to use from many threads.
- **Layouts:** a pattern of fields such as `{time} {level} {message}`, or JSON lines.

Not in 0.1.0:

- Logging on a background thread.
- Removing levels at compile time.
- Key-value fields beside the message.
- syslog, journald, the Windows event log and network sinks.
- Configuration files, and hierarchies of loggers.
- printf-style formatting and wide strings.

## 2. The package

| | |
|---|---|
| Repository | `github.com/cpm-org/cpm-log`, MIT |
| Registry name | `cpm-log` |
| Header | `<cpm/log.hpp>`, which includes the parts in `<cpm/log/*.hpp>` |
| Namespace | `cpm::log` |
| Standard | `"std": "c++20"` |
| Type | `static-lib` |
| Tests | `tests/*.test.cpp` on cpm-test, run with `cpm test`. cpm-json, also a devDependency, checks the JSON layout's output. |
| CI | Linux (GCC and Clang), macOS (Apple Clang) and Windows (MinGW GCC), with CPM installed by its installer: strict warnings as errors, release, and AddressSanitizer with UBSan (and a ThreadSanitizer run) on Linux |

It depends on nothing but the standard library.

`cpm::log` shares its last name with the C function `log`, so `namespace log = cpm::log;` at global scope clashes with `<cmath>`. The README shows `cpm::log::info(...)`, or another alias such as `namespace logging = cpm::log;`.

## 3. Levels

```cpp
enum class Level { trace, debug, info, warning, error, critical, off };
```

- Levels are ordered as written. A logger or sink at level L takes records at L and above, and `off` takes none.
- `to_string(level)` gives the lowercase name: `"trace"`, …, `"critical"`, `"off"`.
- `parse_level(text)` returns `std::optional<Level>`. It reads the names in any case, and `warn` as `warning`. Programs use it to read a level from an environment variable or a configuration.

## 4. Logging

### 4.1 Loggers

```cpp
auto file = std::make_shared<cpm::log::FileSink>("logs/server.log");
cpm::log::Logger net("net", {file}, cpm::log::Level::debug);

net.info("listening on {}:{}", host, port);
net.warning("slow response from {} ({} ms)", peer, ms);
net.log(cpm::log::Level::error, "lost {} connections", lost);
```

A `Logger` has:
- `name()`;
- `level()` and `set_level(level)`, default `info`;
- `enabled(level)`: whether a record at `level` passes the logger's level;
- `sinks()` and `set_sinks(sinks)`: `std::vector<std::shared_ptr<Sink>>`. Sinks can be shared between loggers;
- `trace`, `debug`, `info`, `warning`, `error`, `critical`, and `log(level, ...)`;
- `flush()`, which flushes each sink;
- `set_error_handler(handler)` (§4.4).

Loggers can't be copied or moved, since sinks and other threads refer to them. Every member is safe to call from many threads at once. `set_sinks` takes effect for later records, while records being written finish with the old sinks.

### 4.2 Messages and where they come from

The logging functions take a format string and its arguments, as `std::format` does:

```cpp
template <class... Args>
void info(Format<std::type_identity_t<Args>...> format, Args&&... args);
```

- `Format<Args...>` is built implicitly from the string literal at the call. Its `consteval` constructor checks the format string against the arguments, as `std::format_string` does, so a mismatch doesn't compile. A default argument of the constructor captures `std::source_location::current()`, which is the caller's location. No macros are needed.
- The message is formatted only when the logger's level allows it. The arguments are evaluated anyway, as for any function call, so expensive arguments belong inside `if (logger.enabled(Level::debug))`.
- Logging at `Level::off` does nothing.

### 4.3 Records

What sinks receive:

```cpp
struct Record {
    std::chrono::system_clock::time_point time;
    Level level;
    std::string_view logger;   // the logger's name
    std::string_view message;  // formatted
    std::source_location where;
    std::uint64_t thread;      // 1 for the first thread that logs, 2 for the next, and so on
};
```

The views are valid only during the call to the sink. Thread numbers are small and stable for the life of a thread. They are more readable than `std::thread::id`, which C++20 can't format.

### 4.4 Errors while logging

Logging never throws:
- When formatting the message throws (a user type's formatter, or memory), the record is dropped.
- When a sink throws, the other sinks still get the record.

Either way, the exception goes to the logger's error handler, a `std::function<void(const std::exception&)>`. The default handler writes `cpm-log: logger "<name>": <what>` to standard error the first time, and stays quiet afterwards, so a full disk doesn't flood the console. An error handler must not throw.

### 4.5 The default logger

`default_logger()` returns a process-wide logger:
- its name is `main`, and its level is `info`;
- it has one `ConsoleSink` on standard error with automatic colors.

The free functions `cpm::log::trace(...)` to `critical(...)` and `cpm::log::log(level, ...)` log to it. Programs configure it with `set_level` and `set_sinks`, like any logger.

## 5. Sinks

```cpp
class Sink {
public:
    virtual ~Sink() = default;
    virtual void write(const Record& record) = 0;
    virtual void flush() = 0;
};
```

`write` and `flush` may be called from many threads at once, and implementations make that safe.

### 5.1 Text sinks

The built-in sinks except `CallbackSink` derive from `TextSink`, which:
- keeps a level of its own (`level()`, `set_level()`, default `trace`), so one logger can send everything to a file and only warnings to the console;
- turns a record into a line with its `Layout` (§6), the default `PatternLayout` unless one is given, and adds `\n`;
- writes the line under a lock, so lines from different threads never interleave.

| Sink | Writes to |
|---|---|
| `ConsoleSink(Console stream = Console::standard_error, Color color = Color::automatic, layout)` | standard error or standard output, flushed after every line. `ConsoleSink(std::FILE*, color, layout)` takes any C stream. |
| `StreamSink(std::ostream&, layout)` | a stream, flushed on `flush()` |
| `FileSink(path, FileOptions, layout)` | a file (§5.2) |
| `RotatingFileSink(path, RotationOptions, layout)` | a file that rolls over at a size (§5.3) |
| `MemorySink(layout)` | a list of lines: `lines()` returns a copy and `clear()` empties it. For tests. |

**Colors.** With `Color::automatic`, a `ConsoleSink` colors its lines when the stream is a terminal and the `NO_COLOR` environment variable isn't set. `Color::always` and `Color::never` decide for it. Colors are by level, for the whole line:

| Level | Color |
|---|---|
| `trace`, `debug` | dim (`ESC[90m`) |
| `info` | none |
| `warning` | yellow (`ESC[33m`) |
| `error` | red (`ESC[31m`) |
| `critical` | bold red (`ESC[1;31m`) |

On Windows, a sink that colors turns on the console's escape sequence processing when it is made.

### 5.2 Files

```cpp
struct FileOptions {
    bool append = true;                  // false: start the file empty
    Level flush_level = Level::warning;  // flush after records at this level and above
};
```

- The parent directories are created.
- A file that can't be opened throws `cpm::log::Error` from the constructor.
- The sink flushes after records at `flush_level` and above, on `flush()`, and when it is destroyed.

### 5.3 Rotating files

```cpp
struct RotationOptions {
    std::uintmax_t max_bytes = 10 * 1024 * 1024;
    unsigned max_files = 5;              // rotated files kept beside the current one
    Level flush_level = Level::warning;
};
```

When a line would make the file larger than `max_bytes`, and the file isn't empty, the sink rotates before writing the line:
1. It deletes the file numbered `max_files`.
2. It renames each numbered file to the next number.
3. It renames the current file to number 1.
4. It starts a new, empty file.

Numbers go before the extension: `server.log`, `server.1.log`, `server.2.log`. A name without an extension gets them at the end: `server.1`. With `max_files = 0`, the file is truncated instead.

The size counts the existing file when the sink opens it in append mode. A single line longer than `max_bytes` is still written, to a file of its own.

### 5.4 Callbacks

`CallbackSink(std::function<void(const Record&)> callback, Level level = Level::trace)` passes records at `level` and above to the callback, one call at a time. It is the way to forward records to another system.

## 6. Layouts

```cpp
class Layout {
public:
    virtual ~Layout() = default;
    // Appends the record as one line, without a line break.
    virtual void format(const Record& record, std::string& out) const = 0;
};
```

Layouts are immutable, so one can be shared by several sinks (`std::shared_ptr<const Layout>`).

### 6.1 Patterns

`PatternLayout(std::string_view pattern = default_pattern)` writes the pattern's text, with fields in braces replaced:

| Field | Gives |
|---|---|
| `{time}` | local date and time with milliseconds: `2026-10-26 14:03:07.123` |
| `{utc}` | UTC in ISO 8601: `2026-10-26T12:03:07.123Z` |
| `{level}` | `trace` … `critical` |
| `{logger}` | the logger's name |
| `{message}` | the message |
| `{file}` | the source file's name, without its directories |
| `{path}` | the source file, as the compiler named it |
| `{line}` | the source line |
| `{function}` | the function, as the compiler names it |
| `{thread}` | the thread's number (§4.3) |

- `default_pattern` is `"{time} {level:<8} {logger}: {message}"`, which gives `2026-10-26 14:03:07.123 warning  net: slow response from 10.0.0.7 (840 ms)`.
- A field takes a `std::format` specification after a colon, applied to its text or number: `{level:<8}`, `{line:>4}`, `{thread:03}`. `{time}` and `{utc}` take none.
- `{{` and `}}` are literal braces.
- The pattern is checked once, by the constructor. An unknown field, a brace that isn't closed or matched, or a specification its field can't take throws `cpm::log::Error`, naming the problem and its position in the pattern.
- Messages are written as they are: a layout doesn't escape or check them.

### 6.2 JSON lines

`JsonLayout` writes each record as one JSON object, with the members in this order:

```json
{"time":"2026-10-26T12:03:07.123Z","level":"warning","logger":"net","message":"slow response from 10.0.0.7 (840 ms)","file":"src/net.cpp","line":88,"function":"void poll()","thread":3}
```

- The time is UTC, as for `{utc}`.
- `file` is the path as the compiler named it.
- Strings are escaped as JSON requires: `"` and `\`, control characters as `\n`, `\t` and so on, or `\u00XX`.
- Bytes that aren't valid UTF-8 become U+FFFD, so every line is valid JSON whatever the message holds.

## 7. Time

- **Local time** comes from the platform's thread-safe conversion: `localtime_r`, or `localtime_s` on Windows.
- **UTC** is computed from the time point's days since 1970, with the civil calendar arithmetic, so it doesn't depend on the platform.
- Both truncate to milliseconds.

## 8. Errors

`cpm::log::Error` derives from `std::runtime_error`. It is thrown for configuration problems, never while logging:
- a pattern that isn't valid;
- a file that can't be opened;
- options that can't work, such as `max_bytes = 0`.

## 9. Structure

Each part has one job and sits behind a small interface. Platform differences live in two parts.

| Part | Job |
|---|---|
| `level` | names and parsing |
| `format` | `Format`: the checked format string and the caller's location, in the header |
| `record` | `Record`, in the header |
| `thread_number` | small per-thread numbers |
| `logger` | the level check, formatting the message, delivery to sinks, error handling |
| `default_logger` | the process-wide logger and the free functions |
| `pattern` | parses a pattern into literal text and fields with their specifications |
| `pattern_layout` | writes a record through a parsed pattern |
| `json_layout` | writes a record as a JSON object, with escaping and UTF-8 repair |
| `utf8` | finds the well-formed UTF-8 sequences, for `json_layout` |
| `civil_time` | a time point to its UTC calendar date and time of day |
| `local_time` | a time point to local date and time (platform) |
| `time_text` | the `{time}` and `{utc}` text |
| `text_sink` | level filter, layout and lock for the text sinks |
| `terminal` | whether a stream is a terminal, `NO_COLOR`, and Windows console escape sequences (platform) |
| `console_sink`, `stream_sink`, `file_sink`, `memory_sink`, `callback_sink` | one sink each |
| `log_file` | opening, writing and flushing log files, for both file sinks |
| `rotation` | rotated file names, and the rotation steps |
| `rotating_file_sink` | the size check, using `rotation` |

## 10. Testing

- **Each part** has its own tests.
- **Patterns:** every field, specifications, and every error with its position.
- **UTC:** dates across leap years, centuries and 1970, against known values.
- **JSON:** layout output is parsed back with cpm-json, including messages with quotes, control characters and invalid UTF-8.
- **Source location:** the line a test logs from is the line the record reports.
- **Laziness:** an argument whose formatter counts its calls is never formatted when the level filters it out.
- **Threads:** many threads log to a `MemorySink`, and every line arrives whole. CI runs this under ThreadSanitizer on Linux.
- **Errors:** a throwing sink and a throwing formatter reach the error handler, and other sinks still get the record.
- **Rotation:** in a temporary directory, with small sizes: names, order, `max_files = 0`, appending to an existing file, and a line longer than `max_bytes`.
- **Console:** colors and their absence, through `ConsoleSink` on a temporary `FILE*`.
- **Example project:** `examples/consumer` uses cpm-log through a path dependency, as users do.
