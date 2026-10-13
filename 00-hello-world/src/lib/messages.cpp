#include "dp_hello/messages.hpp"

namespace dp_hello {

std::string format_usage_error(const UsageError& error) {
  std::string text;
  text.append(kToolName).append(": ").append(error.message).append("\n");
  text.append("Try '").append(kToolName).append(" --help' for more information.\n");
  return text;
}

}  // namespace dp_hello
