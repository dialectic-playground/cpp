// Validation (FR-03, FR-05, FR-09): turns ParsedArguments into a GreetingRequest.
#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "dp_hello/cli.hpp"
#include "dp_hello/usage_error.hpp"

namespace dp_hello {

inline constexpr std::string_view kDefaultName = "World";
inline constexpr int kMinimumCount = 1;
inline constexpr int kMaximumCount = 1000;

/// A validated greeting request: `name` is non-empty and `count` is within 1-1000.
struct GreetingRequest {
  std::string name;
  int count = kMinimumCount;

  bool operator==(const GreetingRequest&) const = default;
};

using ValidationResult = std::variant<GreetingRequest, UsageError>;

/// Parses COUNT: ASCII digits only (leading zeros allowed) with a value of 1-1000.
/// Returns std::nullopt for anything else, without overflowing on long inputs.
[[nodiscard]] std::optional<int> parse_count(std::string_view text);

/// Checks, in order, operands (FR-09), the winning name (FR-03), and the winning count (FR-05).
[[nodiscard]] ValidationResult validate_arguments(const ParsedArguments& arguments);

}  // namespace dp_hello
