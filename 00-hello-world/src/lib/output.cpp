#include "dp_hello/output.hpp"

#include <cstdio>
#include <string_view>

namespace dp_hello {
namespace {

bool write_bytes(std::FILE* stream, std::string_view bytes) {
  if (bytes.empty()) {
    return true;
  }
  const bool written = std::fwrite(bytes.data(), 1, bytes.size(), stream) == bytes.size();
  return std::fflush(stream) == 0 && written;
}

}  // namespace

int emit(const Outcome& outcome) {
  // A failed write means the reader has gone away (CONV-25): stop quietly.
  if (write_bytes(stdout, outcome.standard_output)) {
    write_bytes(stderr, outcome.standard_error);
  }
  return outcome.exit_status;
}

}  // namespace dp_hello
