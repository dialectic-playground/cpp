// Unit tests for option parsing (CONV-07 ... CONV-13, FR-02, FR-06, FR-07, FR-08, FR-10).
#include <array>
#include <initializer_list>
#include <string>
#include <variant>
#include <vector>

#include "dp_hello/cli.hpp"
#include "test_support.hpp"

namespace {

using dp_hello::Action;
using dp_hello::OptionValue;
using dp_hello::ParsedArguments;
using dp_hello::ParseResult;
using dp_hello::UsageError;
using dp_hello::testing::Checker;
using dp_hello::testing::TestCase;

ParseResult parse(std::initializer_list<const char*> tokens) {
  const std::vector<std::string> arguments(tokens.begin(), tokens.end());
  return dp_hello::parse_arguments(arguments);
}

ParsedArguments parsed_or_empty(const ParseResult& result) {
  const auto* parsed = std::get_if<ParsedArguments>(&result);
  return parsed != nullptr ? *parsed : ParsedArguments{Action::greet, {}, {}, {"<error>"}};
}

bool is_error(const ParseResult& result, const std::string& message) {
  const auto* error = std::get_if<UsageError>(&result);
  return error != nullptr && error->message == message;
}

void test_no_arguments(Checker& checker) {
  DP_CHECK(checker, parsed_or_empty(parse({})) == ParsedArguments{});
}

void test_name_forms(Checker& checker) {
  const OptionValue long_ada{"Ada", "--name"};
  const OptionValue short_ada{"Ada", "-n"};
  DP_CHECK(checker, parsed_or_empty(parse({"--name", "Ada"})).name == long_ada);
  DP_CHECK(checker, parsed_or_empty(parse({"--name=Ada"})).name == long_ada);
  DP_CHECK(checker, parsed_or_empty(parse({"-n", "Ada"})).name == short_ada);
  DP_CHECK(checker, parsed_or_empty(parse({"-nAda"})).name == short_ada);
}

void test_name_values_starting_with_dash(Checker& checker) {
  DP_CHECK(checker, (parsed_or_empty(parse({"--name=-x"})).name == OptionValue{"-x", "--name"}));
  DP_CHECK(checker, (parsed_or_empty(parse({"--name", "-x"})).name == OptionValue{"-x", "--name"}));
  DP_CHECK(checker, (parsed_or_empty(parse({"-n-x"})).name == OptionValue{"-x", "-n"}));
  DP_CHECK(checker, (parsed_or_empty(parse({"-n", "--"})).name == OptionValue{"--", "-n"}));
}

void test_empty_and_inline_values(Checker& checker) {
  DP_CHECK(checker, (parsed_or_empty(parse({"--name="})).name == OptionValue{"", "--name"}));
  DP_CHECK(checker, (parsed_or_empty(parse({"-n", ""})).name == OptionValue{"", "-n"}));
  DP_CHECK(checker, (parsed_or_empty(parse({"--name=a=b"})).name == OptionValue{"a=b", "--name"}));
}

void test_bundled_short_options(Checker& checker) {
  const ParsedArguments parsed = parsed_or_empty(parse({"-c2", "-nKen"}));
  DP_CHECK(checker, (parsed.count == OptionValue{"2", "-c"}));
  DP_CHECK(checker, (parsed.name == OptionValue{"Ken", "-n"}));
  // A value-taking option consumes the rest of its token (CONV-07).
  DP_CHECK(checker, (parsed_or_empty(parse({"-cn"})).count == OptionValue{"n", "-c"}));
  DP_CHECK(checker, !parsed_or_empty(parse({"-cn"})).name.has_value());
}

void test_last_occurrence_wins(Checker& checker) {
  const ParsedArguments parsed = parsed_or_empty(parse({"-n", "A", "--name=B", "-c", "x", "-c5"}));
  DP_CHECK(checker, (parsed.name == OptionValue{"B", "--name"}));
  DP_CHECK(checker, (parsed.count == OptionValue{"5", "-c"}));
}

void test_operands(Checker& checker) {
  DP_CHECK(checker,
           parsed_or_empty(parse({"extra"})).operands == std::vector<std::string>{"extra"});
  DP_CHECK(checker, parsed_or_empty(parse({"-"})).operands == std::vector<std::string>{"-"});
  DP_CHECK(checker, (parsed_or_empty(parse({"--", "-n", "--help"})).operands ==
                     std::vector<std::string>{"-n", "--help"}));
  DP_CHECK(checker, (parsed_or_empty(parse({"a", "-n", "Ada", "b"})).operands ==
                     std::vector<std::string>{"a", "b"}));
}

void test_help_and_version_stop_the_scan(Checker& checker) {
  DP_CHECK(checker, parsed_or_empty(parse({"--help"})).action == Action::help);
  DP_CHECK(checker, parsed_or_empty(parse({"--version"})).action == Action::version);
  DP_CHECK(checker, parsed_or_empty(parse({"--help", "extra", "-x"})).action == Action::help);
  DP_CHECK(checker, parsed_or_empty(parse({"--version", "--help"})).action == Action::version);
  DP_CHECK(checker, parsed_or_empty(parse({"extra", "--help"})).action == Action::help);
  DP_CHECK(checker, is_error(parse({"-x", "--help"}), "invalid option -- 'x'"));
}

void test_unknown_options(Checker& checker) {
  DP_CHECK(checker, is_error(parse({"-x"}), "invalid option -- 'x'"));
  DP_CHECK(checker, is_error(parse({"-nx", "-y"}), "invalid option -- 'y'"));
  DP_CHECK(checker, is_error(parse({"--nme", "Ada"}), "unrecognized option '--nme'"));
  DP_CHECK(checker, is_error(parse({"--nam"}), "unrecognized option '--nam'"));
  DP_CHECK(checker, is_error(parse({"--names=x"}), "unrecognized option '--names=x'"));
  DP_CHECK(checker, is_error(parse({"--=x"}), "unrecognized option '--=x'"));
  DP_CHECK(checker, is_error(parse({"-h"}), "invalid option -- 'h'"));
  // A non-ASCII option letter is reported as a whole UTF-8 character.
  DP_CHECK(checker, is_error(parse({"-\xC3\xA9"}), "invalid option -- '\xC3\xA9'"));
}

void test_missing_arguments(Checker& checker) {
  DP_CHECK(checker, is_error(parse({"-n"}), "option requires an argument -- 'n'"));
  DP_CHECK(checker, is_error(parse({"-c"}), "option requires an argument -- 'c'"));
  DP_CHECK(checker, is_error(parse({"--name"}), "option '--name' requires an argument"));
  DP_CHECK(checker, is_error(parse({"--count"}), "option '--count' requires an argument"));
}

void test_flags_reject_arguments(Checker& checker) {
  DP_CHECK(checker, is_error(parse({"--help=1"}), "option '--help' doesn't allow an argument"));
  DP_CHECK(checker,
           is_error(parse({"--version=1"}), "option '--version' doesn't allow an argument"));
  DP_CHECK(checker,
           is_error(parse({"--version="}), "option '--version' doesn't allow an argument"));
}

constexpr std::array kTests{
    TestCase{"no_arguments", test_no_arguments},
    TestCase{"name_forms", test_name_forms},
    TestCase{"name_values_starting_with_dash", test_name_values_starting_with_dash},
    TestCase{"empty_and_inline_values", test_empty_and_inline_values},
    TestCase{"bundled_short_options", test_bundled_short_options},
    TestCase{"last_occurrence_wins", test_last_occurrence_wins},
    TestCase{"operands", test_operands},
    TestCase{"help_and_version_stop_the_scan", test_help_and_version_stop_the_scan},
    TestCase{"unknown_options", test_unknown_options},
    TestCase{"missing_arguments", test_missing_arguments},
    TestCase{"flags_reject_arguments", test_flags_reject_arguments},
};

}  // namespace

int main() { return dp_hello::testing::run_tests(kTests); }
