#include "dp_hello/app.hpp"

#include <utility>
#include <variant>

#include "dp_hello/cli.hpp"
#include "dp_hello/greeting.hpp"
#include "dp_hello/messages.hpp"
#include "dp_hello/validation.hpp"

namespace dp_hello {
namespace {

Outcome usage_failure(const UsageError& error) {
  return Outcome{"", format_usage_error(error), kExitUsage};
}

Outcome success(std::string standard_output) {
  return Outcome{std::move(standard_output), "", kExitSuccess};
}

Outcome greet(const ParsedArguments& arguments) {
  const ValidationResult validated = validate_arguments(arguments);
  if (const auto* error = std::get_if<UsageError>(&validated)) {
    return usage_failure(*error);
  }
  const auto& request = std::get<GreetingRequest>(validated);
  return success(build_greeting(request.name, request.count));
}

}  // namespace

Outcome run(std::span<const std::string> arguments) {
  const ParseResult parsed = parse_arguments(arguments);
  if (const auto* error = std::get_if<UsageError>(&parsed)) {
    return usage_failure(*error);
  }
  const auto& parsed_arguments = std::get<ParsedArguments>(parsed);
  switch (parsed_arguments.action) {
    case Action::help:
      return success(std::string(kHelpText));
    case Action::version:
      return success(std::string(kVersionText));
    case Action::greet:
      break;
  }
  return greet(parsed_arguments);
}

}  // namespace dp_hello
