// Output: writes an Outcome to the standard streams as raw bytes (CONV-03, CONV-25).
#pragma once

#include "dp_hello/app.hpp"

namespace dp_hello {

/// Writes the outcome's standard output and standard error bytes, then returns its exit
/// status. A failed write (for example a closed pipe) ends the output silently (CONV-25).
[[nodiscard]] int emit(const Outcome& outcome);

}  // namespace dp_hello
