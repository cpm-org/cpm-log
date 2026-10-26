// The exception for configuration problems (docs/spec.md §8). Logging itself never throws.
#pragma once

#include <stdexcept>

namespace cpm::log {

// A pattern that isn't valid, a file that can't be opened, or options that can't work.
class Error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

}  // namespace cpm::log
