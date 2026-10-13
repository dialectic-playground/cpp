#include "dp_hello/platform.hpp"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>

#include <cstdio>
#include <string_view>

#include "dp_hello/encoding.hpp"
#endif

namespace dp_hello {

void use_binary_standard_streams() {
#ifdef _WIN32
  // Text mode would turn every LF into CRLF (CONV-03).
  _setmode(_fileno(stdout), _O_BINARY);
  _setmode(_fileno(stderr), _O_BINARY);
#endif
}

std::vector<std::string> arguments_from_native(int argc, char* argv[]) {
  std::vector<std::string> arguments;
  for (int index = 1; index < argc; ++index) {
    arguments.emplace_back(argv[index]);
  }
  return arguments;
}

#ifdef _WIN32
std::vector<std::string> arguments_from_wide(int argc, wchar_t* argv[]) {
  static_assert(sizeof(wchar_t) == sizeof(char16_t), "Windows wchar_t is a UTF-16 code unit");
  std::vector<std::string> arguments;
  for (int index = 1; index < argc; ++index) {
    const std::wstring_view wide(argv[index]);
    std::u16string utf16(wide.begin(), wide.end());
    arguments.push_back(utf16_to_utf8(utf16));
  }
  return arguments;
}
#endif

}  // namespace dp_hello
