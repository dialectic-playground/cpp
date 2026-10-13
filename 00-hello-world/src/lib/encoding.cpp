#include "dp_hello/encoding.hpp"

namespace dp_hello {
namespace {

constexpr char32_t kHighSurrogateFirst = 0xD800;
constexpr char32_t kLowSurrogateFirst = 0xDC00;
constexpr char32_t kSurrogateEnd = 0xE000;

bool is_high_surrogate(char32_t unit) {
  return unit >= kHighSurrogateFirst && unit < kLowSurrogateFirst;
}

bool is_low_surrogate(char32_t unit) { return unit >= kLowSurrogateFirst && unit < kSurrogateEnd; }

void append_utf8(std::string& output, char32_t code_point) {
  const auto byte = [&output](char32_t value) { output.push_back(static_cast<char>(value)); };
  if (code_point < 0x80) {
    byte(code_point);
  } else if (code_point < 0x800) {
    byte(0xC0 | (code_point >> 6));
    byte(0x80 | (code_point & 0x3F));
  } else if (code_point < 0x10000) {
    byte(0xE0 | (code_point >> 12));
    byte(0x80 | ((code_point >> 6) & 0x3F));
    byte(0x80 | (code_point & 0x3F));
  } else {
    byte(0xF0 | (code_point >> 18));
    byte(0x80 | ((code_point >> 12) & 0x3F));
    byte(0x80 | ((code_point >> 6) & 0x3F));
    byte(0x80 | (code_point & 0x3F));
  }
}

}  // namespace

std::string utf16_to_utf8(std::u16string_view text) {
  std::string output;
  output.reserve(text.size() * 3);
  for (std::size_t index = 0; index < text.size(); ++index) {
    const char32_t unit = text[index];
    if (is_high_surrogate(unit) && index + 1 < text.size() && is_low_surrogate(text[index + 1])) {
      const char32_t low = text[++index];
      append_utf8(output,
                  0x10000 + ((unit - kHighSurrogateFirst) << 10) + (low - kLowSurrogateFirst));
    } else {
      append_utf8(output, unit);
    }
  }
  return output;
}

std::size_t utf8_sequence_length(std::string_view text) {
  if (text.empty()) {
    return 0;
  }
  const auto lead = static_cast<unsigned char>(text.front());
  std::size_t length = 1;
  if (lead >= 0xF0 && lead <= 0xF7) {
    length = 4;
  } else if (lead >= 0xE0) {
    length = lead <= 0xEF ? 3 : 1;
  } else if (lead >= 0xC0) {
    length = 2;
  }
  return length < text.size() ? length : text.size();
}

}  // namespace dp_hello
