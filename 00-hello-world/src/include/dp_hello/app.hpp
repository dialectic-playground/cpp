// Application logic: maps an argument list to the bytes to write and the exit status.
#pragma once

#include <span>
#include <string>

namespace dp_hello {

inline constexpr int kExitSuccess = 0;
inline constexpr int kExitUsage = 2;

/// Everything one invocation produces. Exactly one of the two streams is non-empty.
struct Outcome {
  std::string standard_output;
  std::string standard_error;
  int exit_status = kExitSuccess;

  bool operator==(const Outcome&) const = default;
};

/// Runs dp-hello on `arguments` (UTF-8, without the program name) without performing I/O.
[[nodiscard]] Outcome run(std::span<const std::string> arguments);

}  // namespace dp_hello
