// Entry point: decodes the platform's arguments, runs dp-hello, and writes the outcome.
#include <exception>
#include <string>
#include <vector>

#include "dp_hello/app.hpp"
#include "dp_hello/output.hpp"
#include "dp_hello/platform.hpp"

namespace {

// Exit status if the C++ runtime itself fails (for example std::bad_alloc). dp-hello has no
// other runtime failure, and nothing escapes main as a crash (CONV-24).
constexpr int kExitInternalFailure = 1;

template <typename DecodeArguments>
int run_main(DecodeArguments decode_arguments) noexcept {
  try {
    dp_hello::use_binary_standard_streams();
    const std::vector<std::string> arguments = decode_arguments();
    return dp_hello::emit(dp_hello::run(arguments));
  } catch (...) {
    return kExitInternalFailure;
  }
}

}  // namespace

#ifdef _WIN32
// MinGW-w64 selects this entry point with -municode: the arguments arrive as UTF-16.
int wmain(int argc, wchar_t* argv[]) {
  return run_main([argc, argv] { return dp_hello::arguments_from_wide(argc, argv); });
}
#else
int main(int argc, char* argv[]) {
  return run_main([argc, argv] { return dp_hello::arguments_from_native(argc, argv); });
}
#endif
