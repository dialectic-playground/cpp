# 00-hello-world: Standard Terminal Hello World, in C++20 with CMake and CTest.
#
# Extends the universal build contract in shared/templates/BASE-MAKEFILE:
#
#   make all | install | uninstall | test | check | lint | format | clean | distclean
#   make test-hidden | grade | help
#
# Container-first: goals run in the toolchain container built from the repository's
# Containerfile (podman, then docker, then local; override with DP_RUNTIME). Use
# DP_RUNTIME=local for native builds (G++ on Linux, Apple Clang on macOS, MinGW-w64 G++ on
# Windows under Git Bash). The container and native builds use separate CMake build trees.

PROJECT := 00-hello-world
TOOLS   := dp-hello

# Path to the sdlc-curriculum checkout (the specs, test suites, and runner live there).
CURRICULUM_ROOT ?= ../sdlc-curriculum

CMAKE        ?= cmake
CTEST        ?= ctest
CLANG_FORMAT ?= clang-format
BUILD_DIR    := build/$(if $(DP_IN_CONTAINER),container,local)
# Windows: MinGW-w64 G++ with its own make, never MSVC (the default generator there).
ifeq ($(OS),Windows_NT)
CMAKE_GENERATOR_ARGS := -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++
endif
SOURCES := $(wildcard src/app/*.cpp src/lib/*.cpp src/include/dp_hello/*.hpp \
	src/tests/*.cpp src/tests/*.hpp)

# Directory that contains the built executables after `make all`.
BIN_DIR := $(BUILD_DIR)/bin

define DP_BUILD
$(CMAKE) -S src -B $(BUILD_DIR) $(CMAKE_GENERATOR_ARGS) -DCMAKE_BUILD_TYPE=Release
$(CMAKE) --build $(BUILD_DIR) --parallel
endef

define DP_INSTALL
mkdir -p "$(BINDIR)"
cp "$(BIN_DIR)/$(TOOLS)$(EXE)" "$(BINDIR)/$(TOOLS)$(EXE)"
endef

DP_UNINSTALL = rm -f "$(BINDIR)/$(TOOLS)$(EXE)"

define DP_UNIT_TEST
$(DP_BUILD)
$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure
endef

# The build compiles with -Wall -Wextra -Wpedantic -Werror (src/CMakeLists.txt).
define DP_LINT
$(DP_BUILD)
$(CLANG_FORMAT) --dry-run --Werror $(SOURCES)
endef

DP_FORMAT    = $(CLANG_FORMAT) -i $(SOURCES)
DP_CLEAN     = rm -rf $(BUILD_DIR)
DP_DISTCLEAN = rm -rf build

include $(CURRICULUM_ROOT)/shared/templates/BASE-MAKEFILE
