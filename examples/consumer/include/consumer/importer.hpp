#pragma once

#include <cpm/log.hpp>

#include <string>
#include <vector>

// Imports "name=value" rows, logging each one at debug and each row it skips as a warning. Returns how many it
// imported.
int import_rows(const std::vector<std::string>& rows, cpm::log::Logger& logger);
