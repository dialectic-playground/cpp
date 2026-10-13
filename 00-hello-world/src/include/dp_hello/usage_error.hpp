// A CONV-13 usage error, shared by the parsing and validation units.
#pragma once

#include <string>

namespace dp_hello {

/// A usage error. `message` is the CONV-13 text without the "dp-hello: " prefix.
struct UsageError {
  std::string message;

  bool operator==(const UsageError&) const = default;
};

}  // namespace dp_hello
