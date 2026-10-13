# dp-hello (curriculum project 00-hello-world), C++20.
# Driven by ../Makefile: configure with `cmake -S src -B build/<runtime>`, build, then `ctest`.
cmake_minimum_required(VERSION 3.20)
project(dp_hello VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Every target compiles warning-free under -Wall -Wextra -Wpedantic (`make lint` relies on it).
option(DP_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
add_library(dp_hello_warnings INTERFACE)
target_compile_options(dp_hello_warnings INTERFACE -Wall -Wextra -Wpedantic -Wshadow -Wconversion
                                                   $<$<BOOL:${DP_WARNINGS_AS_ERRORS}>:-Werror>)

# MinGW-w64: link the C++ runtime statically, so dp-hello.exe runs without the MinGW DLLs.
if(MINGW)
  add_link_options(-static)
endif()

add_library(dp_hello_core STATIC
  lib/app.cpp
  lib/cli.cpp
  lib/encoding.cpp
  lib/greeting.cpp
  lib/messages.cpp
  lib/output.cpp
  lib/platform.cpp
  lib/validation.cpp
)
target_include_directories(dp_hello_core PUBLIC include)
target_link_libraries(dp_hello_core PUBLIC dp_hello_warnings)

add_executable(dp-hello app/main.cpp)
target_link_libraries(dp-hello PRIVATE dp_hello_core)
set_target_properties(dp-hello PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(MINGW)
  # wmain receives the UTF-16 command line (CONV-05).
  target_compile_options(dp-hello PRIVATE -municode)
  target_link_options(dp-hello PRIVATE -municode)
endif()

enable_testing()
foreach(test_name IN ITEMS test_app test_cli test_encoding test_greeting test_validation)
  add_executable(${test_name} tests/${test_name}.cpp)
  target_link_libraries(${test_name} PRIVATE dp_hello_core)
  add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
