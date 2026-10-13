// Unit tests for validation (FR-03, FR-04, FR-05, FR-06, FR-09).
#include <array>
#include <initializer_list>
#include <optional>
#include <string>
#include <variant>

#include "dp_hello/validation.hpp"
#include "test_support.hpp"

namespace {

using dp_hello::GreetingRequest;
using dp_hello::OptionValue;
using dp_hello::ParsedArguments;
using dp_hello::UsageError;
using dp_hello::ValidationResult;
using dp_hello::testing::Checker;
using dp_hello::testing::TestCase;

ParsedArguments with_count(const std::string& value, const std::string& spelling = "-c") {
  ParsedArguments arguments;
  arguments.count = OptionValue{value, spelling};
  return arguments;
}

bool is_error(const ValidationResult& result, const std::string& message) {
  const auto* error = std::get_if<UsageError>(&result);
  return error != nullptr && error->message == message;
}

bool is_request(const ValidationResult& result, const GreetingRequest& expected) {
  const auto* request = std::get_if<GreetingRequest>(&result);
  return request != nullptr && *request == expected;
}

void test_count_boundaries(Checker& checker) {
  DP_CHECK(checker, dp_hello::parse_count("0") == std::nullopt);
  DP_CHECK(checker, dp_hello::parse_count("1") == 1);
  DP_CHECK(checker, dp_hello::parse_count("1000") == 1000);
  DP_CHECK(checker, dp_hello::parse_count("1001") == std::nullopt);
  DP_CHECK(checker, dp_hello::parse_count("9999") == std::nullopt);
}

void test_count_leading_zeros(Checker& checker) {
  DP_CHECK(checker, dp_hello::parse_count("007") == 7);
  DP_CHECK(checker, dp_hello::parse_count("0001000") == 1000);
  DP_CHECK(checker, dp_hello::parse_count("000") == std::nullopt);
  DP_CHECK(checker, dp_hello::parse_count("00001001") == std::nullopt);
}

void test_count_non_digits(Checker& checker) {
  for (const char* text : {"", "abc", "+1", "-1", " 1", "1 ", "1.0", "1e3", "0x10", "\xD9\xA1"}) {
    DP_CHECK(checker, dp_hello::parse_count(text) == std::nullopt);
  }
}

void test_count_huge_values_do_not_overflow(Checker& checker) {
  DP_CHECK(checker, dp_hello::parse_count("99999999999999999999999999") == std::nullopt);
  DP_CHECK(checker, dp_hello::parse_count("4294967297") == std::nullopt);
}

void test_defaults(Checker& checker) {
  DP_CHECK(checker, is_request(dp_hello::validate_arguments({}), GreetingRequest{"World", 1}));
}

void test_valid_name_and_count(Checker& checker) {
  ParsedArguments arguments = with_count("3", "--count");
  arguments.name = OptionValue{"Zo\xC3\xAB", "-n"};
  DP_CHECK(checker,
           is_request(dp_hello::validate_arguments(arguments), GreetingRequest{"Zo\xC3\xAB", 3}));
}

void test_empty_name(Checker& checker) {
  ParsedArguments arguments;
  arguments.name = OptionValue{"", "--name"};
  DP_CHECK(checker,
           is_error(dp_hello::validate_arguments(arguments), "invalid argument '' for '--name'"));
  arguments.name = OptionValue{"", "-n"};
  DP_CHECK(checker,
           is_error(dp_hello::validate_arguments(arguments), "invalid argument '' for '-n'"));
}

void test_invalid_count_messages(Checker& checker) {
  DP_CHECK(checker, is_error(dp_hello::validate_arguments(with_count("0", "--count")),
                             "invalid argument '0' for '--count'"));
  DP_CHECK(checker, is_error(dp_hello::validate_arguments(with_count("abc")),
                             "invalid argument 'abc' for '-c'"));
  DP_CHECK(checker, is_error(dp_hello::validate_arguments(with_count("1001")),
                             "invalid argument '1001' for '-c'"));
  DP_CHECK(checker,
           is_error(dp_hello::validate_arguments(with_count("")), "invalid argument '' for '-c'"));
}

void test_operand_reported_first(Checker& checker) {
  ParsedArguments arguments = with_count("0");
  arguments.name = OptionValue{"", "-n"};
  arguments.operands = {"first", "second"};
  DP_CHECK(checker, is_error(dp_hello::validate_arguments(arguments), "extra operand 'first'"));
}

void test_name_checked_before_count(Checker& checker) {
  ParsedArguments arguments = with_count("0");
  arguments.name = OptionValue{"", "-n"};
  DP_CHECK(checker,
           is_error(dp_hello::validate_arguments(arguments), "invalid argument '' for '-n'"));
}

constexpr std::array kTests{
    TestCase{"count_boundaries", test_count_boundaries},
    TestCase{"count_leading_zeros", test_count_leading_zeros},
    TestCase{"count_non_digits", test_count_non_digits},
    TestCase{"count_huge_values_do_not_overflow", test_count_huge_values_do_not_overflow},
    TestCase{"defaults", test_defaults},
    TestCase{"valid_name_and_count", test_valid_name_and_count},
    TestCase{"empty_name", test_empty_name},
    TestCase{"invalid_count_messages", test_invalid_count_messages},
    TestCase{"operand_reported_first", test_operand_reported_first},
    TestCase{"name_checked_before_count", test_name_checked_before_count},
};

}  // namespace

int main() { return dp_hello::testing::run_tests(kTests); }
