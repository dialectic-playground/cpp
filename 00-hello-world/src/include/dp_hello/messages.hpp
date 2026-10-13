// Fixed texts: help (user manual, CONV-23), version (CONV-02), and diagnostics (CONV-13/14).
#pragma once

#include <string>
#include <string_view>

#include "dp_hello/usage_error.hpp"

namespace dp_hello {

inline constexpr std::string_view kToolName = "dp-hello";

inline constexpr std::string_view kHelpText =
    "Usage: dp-hello [OPTION]...\n"
    "Print a greeting to standard output.\n"
    "\n"
    "Options:\n"
    "  -n, --name=NAME     greet NAME instead of World\n"
    "  -c, --count=COUNT   print the greeting COUNT times (1-1000, default 1)\n"
    "      --help          display this help and exit\n"
    "      --version       output version information and exit\n";

inline constexpr std::string_view kVersionText = "dp-hello (dialectic-playground) 1.0.0\n";

/// Formats a usage error as the two CONV-13/CONV-14 lines written to standard error.
[[nodiscard]] std::string format_usage_error(const UsageError& error);

}  // namespace dp_hello
