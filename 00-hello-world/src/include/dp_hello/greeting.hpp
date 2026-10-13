// Greeting construction (NFR-05): pure functions with no I/O.
#pragma once

#include <string>
#include <string_view>

namespace dp_hello {

/// Returns "Hello, " + name + "!" followed by LF. `name` is copied byte for byte.
[[nodiscard]] std::string build_greeting_line(std::string_view name);

/// Returns the greeting line for `name` repeated `count` times (empty when count <= 0).
[[nodiscard]] std::string build_greeting(std::string_view name, int count);

}  // namespace dp_hello
