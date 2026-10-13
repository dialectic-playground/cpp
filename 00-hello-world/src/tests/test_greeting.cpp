// Unit tests for greeting construction (FR-01, FR-02, FR-04, NFR-05).
#include <array>
#include <string>

#include "dp_hello/greeting.hpp"
#include "test_support.hpp"

namespace {

using dp_hello::build_greeting;
using dp_hello::build_greeting_line;
using dp_hello::testing::Checker;
using dp_hello::testing::TestCase;

void test_default_line(Checker& checker) {
  DP_CHECK(checker, build_greeting_line("World") == "Hello, World!\n");
}

void test_name_is_verbatim(Checker& checker) {
  DP_CHECK(checker, build_greeting_line("Ada Lovelace") == "Hello, Ada Lovelace!\n");
  DP_CHECK(checker, build_greeting_line("Zo\xC3\xAB") == "Hello, Zo\xC3\xAB!\n");
  DP_CHECK(checker, build_greeting_line("-x") == "Hello, -x!\n");
  DP_CHECK(checker, build_greeting_line("a\\b 'c' $d") == "Hello, a\\b 'c' $d!\n");
}

void test_repetition(Checker& checker) {
  DP_CHECK(checker, build_greeting("Ken", 1) == "Hello, Ken!\n");
  DP_CHECK(checker, build_greeting("Ken", 2) == "Hello, Ken!\nHello, Ken!\n");
  DP_CHECK(checker, build_greeting("Ken", 0).empty());
}

void test_maximum_count(Checker& checker) {
  const std::string greeting = build_greeting("World", 1000);
  const std::string line = build_greeting_line("World");
  DP_CHECK(checker, greeting.size() == line.size() * 1000);
  DP_CHECK(checker, greeting.substr(greeting.size() - line.size()) == line);
}

void test_lines_end_in_lf_only(Checker& checker) {
  const std::string greeting = build_greeting("World", 3);
  DP_CHECK(checker, greeting.find('\r') == std::string::npos);
  DP_CHECK(checker, greeting.back() == '\n');
}

constexpr std::array kTests{
    TestCase{"default_line", test_default_line},
    TestCase{"name_is_verbatim", test_name_is_verbatim},
    TestCase{"repetition", test_repetition},
    TestCase{"maximum_count", test_maximum_count},
    TestCase{"lines_end_in_lf_only", test_lines_end_in_lf_only},
};

}  // namespace

int main() { return dp_hello::testing::run_tests(kTests); }
