#include "dp_hello/validation.hpp"

#include <cstddef>

namespace dp_hello {
namespace {

UsageError invalid_argument(const OptionValue& option) {
  return UsageError{"invalid argument '" + option.value + "' for '" + option.spelling + "'"};
}

}  // namespace

std::optional<int> parse_count(std::string_view text) {
  if (text.empty()) {
    return std::nullopt;
  }
  for (const char character : text) {
    if (character < '0' || character > '9') {
      return std::nullopt;
    }
  }
  // Skip leading zeros, then reject anything longer than the largest valid count, so the
  // accumulation below cannot overflow.
  const std::size_t first_significant = text.find_first_not_of('0');
  if (first_significant == std::string_view::npos) {
    return std::nullopt;  // all zeros: the value is 0
  }
  const std::string_view significant = text.substr(first_significant);
  constexpr std::size_t kMaximumDigits = 4;  // digits in kMaximumCount
  if (significant.size() > kMaximumDigits) {
    return std::nullopt;
  }
  int value = 0;
  for (const char digit : significant) {
    value = value * 10 + (digit - '0');
  }
  if (value < kMinimumCount || value > kMaximumCount) {
    return std::nullopt;
  }
  return value;
}

ValidationResult validate_arguments(const ParsedArguments& arguments) {
  if (!arguments.operands.empty()) {
    return UsageError{"extra operand '" + arguments.operands.front() + "'"};
  }
  GreetingRequest request{std::string(kDefaultName), kMinimumCount};
  if (arguments.name) {
    if (arguments.name->value.empty()) {
      return invalid_argument(*arguments.name);
    }
    request.name = arguments.name->value;
  }
  if (arguments.count) {
    const std::optional<int> count = parse_count(arguments.count->value);
    if (!count) {
      return invalid_argument(*arguments.count);
    }
    request.count = *count;
  }
  return request;
}

}  // namespace dp_hello
