// Unit tests for UTF-16 to UTF-8 conversion and UTF-8 sequence lengths (CONV-05).
#include <array>
#include <string>

#include "dp_hello/encoding.hpp"
#include "test_support.hpp"

namespace {

using dp_hello::utf16_to_utf8;
using dp_hello::utf8_sequence_length;
using dp_hello::testing::Checker;
using dp_hello::testing::TestCase;

void test_ascii_and_bmp(Checker& checker) {
  DP_CHECK(checker, utf16_to_utf8(u"").empty());
  DP_CHECK(checker, utf16_to_utf8(u"Ada") == "Ada");
  DP_CHECK(checker, utf16_to_utf8(u"Zoë") == "Zo\xC3\xAB");
  DP_CHECK(checker, utf16_to_utf8(u"€") == "\xE2\x82\xAC");
}

void test_surrogate_pairs(Checker& checker) {
  DP_CHECK(checker, utf16_to_utf8(u"\U0001F600") == "\xF0\x9F\x98\x80");
  DP_CHECK(checker, utf16_to_utf8(u"\U0010FFFF") == "\xF4\x8F\xBF\xBF");
}

void test_unpaired_surrogates(Checker& checker) {
  const std::u16string lone_high{static_cast<char16_t>(0xD800), u'x'};
  DP_CHECK(checker, utf16_to_utf8(lone_high) == "\xED\xA0\x80x");
  const std::u16string lone_low{static_cast<char16_t>(0xDC00)};
  DP_CHECK(checker, utf16_to_utf8(lone_low) == "\xED\xB0\x80");
}

void test_sequence_lengths(Checker& checker) {
  DP_CHECK(checker, utf8_sequence_length("") == 0);
  DP_CHECK(checker, utf8_sequence_length("x") == 1);
  DP_CHECK(checker, utf8_sequence_length("\xC3\xA9z") == 2);
  DP_CHECK(checker, utf8_sequence_length("\xE2\x82\xAC") == 3);
  DP_CHECK(checker, utf8_sequence_length("\xF0\x9F\x98\x80") == 4);
  DP_CHECK(checker, utf8_sequence_length("\xF0\x9F") == 2);  // truncated: capped
  DP_CHECK(checker, utf8_sequence_length("\x80") == 1);      // stray continuation byte
  DP_CHECK(checker, utf8_sequence_length("\xFF") == 1);      // invalid lead byte
}

constexpr std::array kTests{
    TestCase{"ascii_and_bmp", test_ascii_and_bmp},
    TestCase{"surrogate_pairs", test_surrogate_pairs},
    TestCase{"unpaired_surrogates", test_unpaired_surrogates},
    TestCase{"sequence_lengths", test_sequence_lengths},
};

}  // namespace

int main() { return dp_hello::testing::run_tests(kTests); }
