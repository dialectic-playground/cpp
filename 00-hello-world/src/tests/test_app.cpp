// Integration tests: argument list to Outcome, without I/O (FR-01 ... FR-11, CONV-12 ... CONV-14).
#include <array>
#include <initializer_list>
#include <string>
#include <vector>

#include "dp_hello/app.hpp"
#include "dp_hello/messages.hpp"
#include "test_support.hpp"

namespace {

using dp_hello::Outcome;
using dp_hello::testing::Checker;
using dp_hello::testing::TestCase;

const std::string kTryHelp = "Try 'dp-hello --help' for more information.\n";

Outcome run(std::initializer_list<const char*> tokens) {
  const std::vector<std::string> arguments(tokens.begin(), tokens.end());
  return dp_hello::run(arguments);
}

Outcome usage_error(const std::string& message) {
  return Outcome{"", "dp-hello: " + message + "\n" + kTryHelp, dp_hello::kExitUsage};
}

Outcome printed(const std::string& output) { return Outcome{output, "", dp_hello::kExitSuccess}; }

void test_greetings(Checker& checker) {
  DP_CHECK(checker, run({}) == printed("Hello, World!\n"));
  DP_CHECK(checker, run({"--name", "Ada Lovelace"}) == printed("Hello, Ada Lovelace!\n"));
  DP_CHECK(checker, run({"-c", "2", "-n", "Ken"}) == printed("Hello, Ken!\nHello, Ken!\n"));
  DP_CHECK(checker, run({"--count=2", "--name", "Ken"}) == printed("Hello, Ken!\nHello, Ken!\n"));
  DP_CHECK(checker, run({"-n", "A", "-n", "B"}) == printed("Hello, B!\n"));
  DP_CHECK(checker, run({"-c", "0", "-c", "1"}) == printed("Hello, World!\n"));
}

void test_help_and_version(Checker& checker) {
  DP_CHECK(checker, run({"--help"}) == printed(std::string(dp_hello::kHelpText)));
  DP_CHECK(checker, run({"--help", "extra", "-x"}) == printed(std::string(dp_hello::kHelpText)));
  DP_CHECK(checker, run({"-n", "", "--help"}) == printed(std::string(dp_hello::kHelpText)));
  DP_CHECK(checker, run({"--version"}) == printed("dp-hello (dialectic-playground) 1.0.0\n"));
}

void test_usage_errors(Checker& checker) {
  DP_CHECK(checker, run({"-x"}) == usage_error("invalid option -- 'x'"));
  DP_CHECK(checker, run({"--nme", "Ada"}) == usage_error("unrecognized option '--nme'"));
  DP_CHECK(checker, run({"--name"}) == usage_error("option '--name' requires an argument"));
  DP_CHECK(checker,
           run({"--version=1"}) == usage_error("option '--version' doesn't allow an argument"));
  DP_CHECK(checker, run({"--name", ""}) == usage_error("invalid argument '' for '--name'"));
  DP_CHECK(checker, run({"-c", "abc"}) == usage_error("invalid argument 'abc' for '-c'"));
  DP_CHECK(checker,
           run({"--count", "1001"}) == usage_error("invalid argument '1001' for '--count'"));
  DP_CHECK(checker, run({"extra"}) == usage_error("extra operand 'extra'"));
  DP_CHECK(checker, run({"--", "-n"}) == usage_error("extra operand '-n'"));
  DP_CHECK(checker, run({"-x", "--help"}) == usage_error("invalid option -- 'x'"));
}

void test_help_text_layout(Checker& checker) {
  const std::string_view help = dp_hello::kHelpText;
  DP_CHECK(checker, help.starts_with("Usage: dp-hello [OPTION]...\n"));
  DP_CHECK(checker, help.ends_with("output version information and exit\n"));
  DP_CHECK(checker, help.find('\r') == std::string_view::npos);
}

constexpr std::array kTests{
    TestCase{"greetings", test_greetings},
    TestCase{"help_and_version", test_help_and_version},
    TestCase{"usage_errors", test_usage_errors},
    TestCase{"help_text_layout", test_help_text_layout},
};

}  // namespace

int main() { return dp_hello::testing::run_tests(kTests); }
