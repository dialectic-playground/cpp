// Minimal assertion helpers for the CTest-registered unit-test executables.
#pragma once

#include <cstdio>
#include <span>
#include <string_view>

namespace dp_hello::testing {

/// Records failed checks; a test executable fails when any check failed.
class Checker {
 public:
  void check(bool condition, std::string_view expression, const char* file, int line) {
    if (!condition) {
      ++failures_;
      std::fprintf(stderr, "%s:%d: check failed: %.*s\n", file, line,
                   static_cast<int>(expression.size()), expression.data());
    }
  }

  [[nodiscard]] int failures() const { return failures_; }

 private:
  int failures_ = 0;
};

struct TestCase {
  std::string_view name;
  void (*body)(Checker&);
};

/// Runs every test case, reports each result, and returns the process exit status.
inline int run_tests(std::span<const TestCase> tests) {
  int failed_tests = 0;
  for (const TestCase& test : tests) {
    Checker checker;
    test.body(checker);
    const bool passed = checker.failures() == 0;
    failed_tests += passed ? 0 : 1;
    std::printf("%s %.*s\n", passed ? "PASS" : "FAIL", static_cast<int>(test.name.size()),
                test.name.data());
  }
  std::printf("%zu tests, %d failed\n", tests.size(), failed_tests);
  return failed_tests == 0 ? 0 : 1;
}

}  // namespace dp_hello::testing

#define DP_CHECK(checker, expression) (checker).check((expression), #expression, __FILE__, __LINE__)
