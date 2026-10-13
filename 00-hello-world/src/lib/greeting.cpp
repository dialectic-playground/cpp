#include "dp_hello/greeting.hpp"

#include <cstddef>

namespace dp_hello {

std::string build_greeting_line(std::string_view name) {
  constexpr std::string_view kPrefix = "Hello, ";
  constexpr std::string_view kSuffix = "!\n";
  std::string line;
  line.reserve(kPrefix.size() + name.size() + kSuffix.size());
  line.append(kPrefix).append(name).append(kSuffix);
  return line;
}

std::string build_greeting(std::string_view name, int count) {
  if (count <= 0) {
    return {};
  }
  const std::string line = build_greeting_line(name);
  std::string greeting;
  greeting.reserve(line.size() * static_cast<std::size_t>(count));
  for (int repetition = 0; repetition < count; ++repetition) {
    greeting.append(line);
  }
  return greeting;
}

}  // namespace dp_hello
