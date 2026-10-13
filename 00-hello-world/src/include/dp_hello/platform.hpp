// Operating-system boundary: argument decoding and binary standard streams (CONV-03, CONV-05).
#pragma once

#include <string>
#include <vector>

namespace dp_hello {

/// Switches standard output and standard error to binary mode on Windows (no CRLF
/// translation). Does nothing on POSIX systems, where streams are always binary.
void use_binary_standard_streams();

/// Returns the arguments after the program name. POSIX arguments are already UTF-8 bytes.
[[nodiscard]] std::vector<std::string> arguments_from_native(int argc, char* argv[]);

#ifdef _WIN32
/// Returns the arguments after the program name, converted from UTF-16 to UTF-8.
[[nodiscard]] std::vector<std::string> arguments_from_wide(int argc, wchar_t* argv[]);
#endif

}  // namespace dp_hello
