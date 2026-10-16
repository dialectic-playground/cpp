<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

**Table of Contents** _generated with [DocToc](https://github.com/thlorenz/doctoc)_

- [cpp](#cpp)
  - [C++ at a glance](#c-at-a-glance)
  - [Toolchain](#toolchain)
  - [Layout](#layout)
  - [Development](#development)
  - [Implementing a project](#implementing-a-project)
  - [Contributing](#contributing)
  - [License](#license)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://raw.githubusercontent.com/dialectic-playground/.github/main/assets/cpp/dist/logo-dark.svg">
    <img alt="dialectic playground / cpp" src="https://raw.githubusercontent.com/dialectic-playground/.github/main/assets/cpp/dist/logo.svg" height="96">
  </picture>
</p>

# cpp

C++ implementation of the Dialectic Playground SDLC curriculum.

📖 **Documentation:** <https://dialectic-playground.github.io/docs/reference/languages/cpp/>

This repository implements the fifty-three projects of the
[SDLC curriculum](https://github.com/dialectic-playground/sdlc-curriculum) in C++. Each project is
specified to the byte by the curriculum and verified by its black-box tests on Linux, macOS,
and Windows.

**Status:** Implemented projects: 00-hello-world

## C++ at a glance

| Aspect                  | C++                                                      |
| :---------------------- | :------------------------------------------------------- |
| File extension          | `.cpp`                                                   |
| CLI classification      | Included                                                 |
| Image category          | Native Toolchain                                         |
| Access to OS primitives | Absolute & Direct (POSIX system calls)                   |
| Entry-point mechanism   | Kernel drops argc and argv pointers directly into main() |

## Toolchain

|                      |                                                                                                    |
| :------------------- | :------------------------------------------------------------------------------------------------- |
| Release line         | C++20 (GCC 16 in the container)                                                                    |
| Build / package tool | CMake                                                                                              |
| Container image      | `gcc:16-trixie`, pinned by digest ([`Containerfile`](Containerfile))                               |
| Native CI            | G++ on Linux, Apple Clang on macOS, MinGW-w64 G++ on Windows, plus CMake                           |
| Unit tests           | CTest, one executable per unit with small assertion helpers (no third-party framework)             |
| Lint                 | `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror` build + `clang-format --dry-run --Werror` |
| Format               | `clang-format` 19 ([`.clang-format`](.clang-format): Google style, 100 columns)                    |

`make toolchain` verifies that the container and native toolchains are on this release line.

clang-format is pinned to the 19 release line everywhere, because formatting output changes
between releases: Debian trixie's `clang-format` in the container and the `clang-format==19.1.7`
PyPI wheel natively (`python -m pip install clang-format==19.1.7` on Linux, macOS, and Windows;
CI does this). Each project builds with CMake into `build/container` or `build/local`, so
container and native builds never share a CMake cache. On Windows, run `make` from Git Bash
with MinGW-w64 G++ and CMake on `PATH`; the projects use the "MinGW Makefiles" generator (never
MSVC), link the C++ runtime statically, and take UTF-16 arguments through `wmain`
(`-municode`).

## Layout

| Path                                 | Contents                                                                                                          |
| :----------------------------------- | :---------------------------------------------------------------------------------------------------------------- |
| `00-hello-world/` … `52-spacecraft/` | One workspace per curriculum project: `Makefile` (with `DP_*` hooks), HLD, LLD, RTM, master test plan, and `src/` |
| `sdlc-curriculum/`                   | The curriculum, as a git submodule pinned to a release tag                                                        |
| `Makefile`                           | `make check` (repository, toolchain, implemented projects), `toolchain`, `projects`, `roundtrip`                  |
| `Containerfile`, `.devcontainer/`    | The toolchain image and the matching dev container                                                                |
| `tools/dp-run`, `tools/container.mk` | Shared container tooling (copies of the canonical files in `dialectic-playground/.github`)                        |
| `.github/workflows/ci.yml`           | CI: container and native (Linux, macOS, Windows) `make` targets, shared-tooling check, DCO                        |

## Development

Container-first: `make` targets and `tools/dp-run <command>` run in the toolchain container
via Podman, then Docker, then the local system. `DP_RUNTIME=podman|docker|local` overrides
the choice.

```sh
git clone --recurse-submodules https://github.com/dialectic-playground/cpp.git
cd cpp
make check                    # repository, toolchain, and every implemented project
DP_RUNTIME=local make check   # the same with your native toolchain
make -C 00-hello-world help   # a project's configuration and hooks
```

## Implementing a project

1. Complete the project's `hld.md` and `lld.md` before writing code.
2. Write the implementation and its unit tests under the project's `src/`, add the
   repository's CMake configuration, and fill in `BIN_DIR` and the `DP_*` hooks in
   the project's `Makefile`.
3. Pass `make -C <project> test` (container) and `DP_RUNTIME=local make -C <project> test`.
4. Add the project directory to `IMPLEMENTED` in the root `Makefile`. CI then builds, lints,
   and tests it in the container and natively on all three operating systems.
5. Keep `rtm.md` and `master-test-plan.md` current.

Specifications: [curriculum reference](https://dialectic-playground.github.io/docs/reference/curriculum/). Language setup:
[Set up a language repository](https://dialectic-playground.github.io/docs/how-to/set-up-a-language-repository/).

## Contributing

See the [organization contributing guide](https://github.com/dialectic-playground/.github/blob/main/CONTRIBUTING.md).
Pull requests are squash-merged, titles follow Conventional Commits, and every commit needs a
DCO sign-off.

## License

[MIT](LICENSE)
