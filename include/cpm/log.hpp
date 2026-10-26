// cpm-log: logging for C++20. See README.md for a guide and docs/spec.md for the specification.
//
//   #include <cpm/log.hpp>
//
//   cpm::log::info("listening on {}:{}", host, port);       // the default logger, on standard error
//
//   auto file = std::make_shared<cpm::log::FileSink>("logs/net.log");
//   cpm::log::Logger net("net", {file}, cpm::log::Level::debug);
//   net.warning("slow response from {} ({} ms)", peer, ms);  // the format string is checked at compile time
#pragma once

#include <cpm/log/callback_sink.hpp>
#include <cpm/log/console_sink.hpp>
#include <cpm/log/default_logger.hpp>
#include <cpm/log/error.hpp>
#include <cpm/log/file_sink.hpp>
#include <cpm/log/format.hpp>
#include <cpm/log/layout.hpp>
#include <cpm/log/level.hpp>
#include <cpm/log/logger.hpp>
#include <cpm/log/memory_sink.hpp>
#include <cpm/log/record.hpp>
#include <cpm/log/sink.hpp>
#include <cpm/log/stream_sink.hpp>
