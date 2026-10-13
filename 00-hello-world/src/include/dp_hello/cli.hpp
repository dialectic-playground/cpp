// Option parsing (CONV-07 ... CONV-13): turns the argument list into ParsedArguments.
#pragma once

#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

#include "dp_hello/usage_error.hpp"

namespace dp_hello {

/// What the invocation asks for, decided by the left-to-right scan (CONV-12).
enum class Action { greet, help, version };

/// An option value exactly as given, with the option as the user spelled it ("-n" or "--name").
struct OptionValue {
  std::string value;
  std::string spelling;

  bool operator==(const OptionValue&) const = default;
};

/// The syntactic result of the scan. Values are not validated yet (FR-06: last one wins).
struct ParsedArguments {
  Action action = Action::greet;
  std::optional<OptionValue> name;
  std::optional<OptionValue> count;
  std::vector<std::string> operands;

  bool operator==(const ParsedArguments&) const = default;
};

using ParseResult = std::variant<ParsedArguments, UsageError>;

/// Scans `arguments` (UTF-8, without the program name) from left to right. Stops at the first
/// --help or --version, or at the first malformed option, which is returned as a UsageError.
[[nodiscard]] ParseResult parse_arguments(std::span<const std::string> arguments);

}  // namespace dp_hello
