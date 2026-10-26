# Changelog

## 0.1.0

The first release.

- **Loggers** with a name, a level and shared sinks, safe to use from many threads, plus a default logger behind `cpm::log::info(...)` and the other free functions.
- **Messages** formatted with `std::format`, with the format string checked at compile time. They are formatted only when the level allows, and the caller's file, line and function are captured without macros.
- **Sinks:** `ConsoleSink` (colored by level, respecting `NO_COLOR`), `FileSink`, `RotatingFileSink`, `StreamSink`, `MemorySink` and `CallbackSink`. Each has a level of its own, and lines never interleave.
- **Layouts:** `PatternLayout` with fields and `std::format` specifications, and `JsonLayout` for JSON lines.
- **Logging never throws:** failures go to an error handler, and the other sinks still get the record.
