# High-Level Design (HLD) — Standard Terminal Hello World

| Field | Value |
| :-- | :-- |
| Project | `00-hello-world` |
| Tools | dp-hello |
| Implementation language | C++20 (CMake, CTest) |
| Author | Brandon Perkins |
| Version / date | 1.0 — 2026-10-13 |
| Status | Approved |
| Inputs | `specs/URD.md`, `specs/SRS.md`, `specs/user-manual.md`, `shared/CLI-CONVENTIONS.md` |

## 1. Introduction

### 1.1 Scope
`dp-hello` writes `Hello, ` + NAME + `!` + LF to standard output COUNT times (NAME defaults to
`World`, COUNT to 1, range 1–1000), prints help and version text, and reports usage errors
with the exact CONV-13/14 diagnostics and exit status 2 (SRS §1.2, §3). It reads no input, no
files, and no environment variables, takes no operands, and starts no processes.

### 1.2 Definitions and acronyms
| Term | Meaning |
| :-- | :-- |
| Outcome | The bytes for stdout and stderr plus the exit status of one invocation |
| Usage error | A CONV-13 condition; reported as two stderr lines, exit status 2 |
| Winning value | The last occurrence of a repeated option (CONV-11) |
| WTF-8 | UTF-8 extended to encode unpaired UTF-16 surrogates as 3-byte sequences |

### 1.3 References
- SRS-00 §3 (interfaces), §4 (FR-01 … FR-11), §5 (NFR-01 … NFR-07), §6 (diagnostics)
- CLI-CONV §2 (CONV-03, CONV-05), §3 (CONV-07 … CONV-12), §4 (CONV-13 … CONV-15), §8

## 2. Requirements summary

| Area | SRS requirements | Architectural impact |
| :-- | :-- | :-- |
| Option parsing | FR-02, FR-06, FR-07, FR-08, FR-10, CONV-07 … CONV-12 | A table-driven scanner (`cli`) that stops at `--help`/`--version` or the first malformed option |
| Validation | FR-03, FR-04, FR-05, FR-09 | A separate unit (`validation`) applied to the winning values only, after the scan |
| Greeting | FR-01, FR-02, FR-04, NFR-05 | Pure functions (`greeting`) with no I/O |
| Diagnostics and fixed texts | FR-07, FR-08, §6, CONV-13 … CONV-15, CONV-23 | One unit (`messages`) owns every literal text |
| Output and atomicity | FR-11, NFR-01, CONV-03, CONV-25 | Logic returns an `Outcome`; only `output` writes, in one pass, after all decisions |
| Platform | NFR-01, CONV-03, CONV-05 | `platform` isolates Windows: UTF-16 arguments and binary streams |

## 3. Architectural overview

### 3.1 Context diagram

```text
     argv (UTF-8 bytes; Windows: UTF-16) ──▶ ┌──────────┐ ──▶ stdout (greeting / help / version)
                                             │ dp-hello │ ──▶ stderr (usage diagnostics)
                                             └──────────┘ ──▶ exit status 0 or 2
     stdin, files, environment, network: not used (CONV-26, NFR-07)
```

### 3.2 Component (module) decomposition

| Component | Responsibility | Depends on |
| :-- | :-- | :-- |
| `app/main.cpp` | Entry point: choose `main`/`wmain`, catch everything, return the exit status | platform, app, output |
| `platform` | Decode arguments to UTF-8 strings; switch stdout/stderr to binary on Windows | encoding |
| `encoding` | UTF-16 → UTF-8 conversion; UTF-8 sequence length (pure) | — |
| `cli` | Scan the arguments into `ParsedArguments` or a `UsageError` | encoding |
| `validation` | Turn `ParsedArguments` into a `GreetingRequest` or a `UsageError` | cli |
| `greeting` | Build the greeting text (pure) | — |
| `messages` | Help text, version line, usage-error formatting | — |
| `app` | Orchestrate cli → validation → greeting/messages into an `Outcome` (no I/O) | cli, validation, greeting, messages |
| `output` | Write an `Outcome` to the standard streams as raw bytes | — |

### 3.3 Data flow
`main` decodes the arguments (`platform`), then calls `app::run`. `run` calls
`parse_arguments`. A `UsageError` becomes an exit-2 `Outcome`. Otherwise `--help`/`--version`
produce their text; a greeting request goes through `validate_arguments` (operands, then the
name, then the count) and `build_greeting`. `emit` writes the `Outcome`'s stdout bytes, then
its stderr bytes, and `main` returns its exit status.

## 4. Key design decisions

| # | Decision | Alternatives considered | Rationale |
| :-- | :-- | :-- | :-- |
| D1 | Build the whole output in memory, then write once | Stream line by line | At most 1000 short lines (a few hundred KiB worst case); one write keeps error atomicity (FR-11) trivial |
| D2 | Errors are values (`std::variant<T, UsageError>`), not exceptions | Exceptions; error codes | Explicit control flow, testable results; C++20 has no `std::expected` |
| D3 | Parse first, validate after the scan | Validate each value as it is read | Last occurrence wins and only the winner is validated (FR-06); `--help` after a bad value still wins (CONV-12) |
| D4 | Pure `run()` returning an `Outcome` | Writing from inside the logic | End-to-end behavior is unit-testable without processes; I/O lives in one unit |
| D5 | `wmain` with `-municode` on MinGW-w64 | `GetCommandLineW` + `CommandLineToArgvW` | The C runtime already splits the UTF-16 command line; no extra library |
| D6 | Own UTF-16 → UTF-8 converter | `WideCharToMultiByte` | Portable and unit-tested on every OS; unpaired surrogates are preserved (WTF-8), never replaced |
| D7 | Hand-written assertion helpers per CTest executable | GoogleTest, Catch2 | The owner's tooling table (CTest + asserts); no third-party dependency to fetch or lock |

## 5. Cross-platform strategy
All Windows differences are isolated in `platform.cpp` and `main.cpp` (`#ifdef _WIN32`).
Every generated line ends in `\n`; `use_binary_standard_streams()` calls
`_setmode(_fileno(stdout|stderr), _O_BINARY)` so the C runtime never adds CR (CONV-03). On
POSIX the arguments are passed through as bytes; on Windows `wmain` receives UTF-16 and
`utf16_to_utf8` converts it (CONV-05). Unknown short options are reported as a whole UTF-8
character, never a partial byte. The tool handles no paths (CONV-19 … CONV-21 do not apply).
The executable is linked statically on MinGW-w64 so it runs without the MinGW DLLs.

## 6. Error handling and diagnostics
Usage errors are `UsageError{message}` values created by `cli` and `validation` with the
CONV-13 text (values inserted verbatim, CONV-15). `app::run` is the only place that maps them
to an `Outcome` (stderr = `messages::format_usage_error`, exit 2). A write failure (EPIPE,
CONV-25) stops output silently; on POSIX, SIGPIPE ends the process. `main` catches any
exception (only `std::bad_alloc` is possible) and returns 1 without a stack trace (CONV-24).

## 7. Non-functional design

| Quality attribute | SRS reference | How the design achieves it |
| :-- | :-- | :-- |
| Performance / memory | NFR-02 | Native executable; no work beyond building one string of O(COUNT × name length) bytes, reserved once |
| Portability | NFR-01 | Binary streams, LF literals, UTF-8 arguments on every OS; CI on Linux, macOS, Windows |
| Robustness | NFR-03 | No unchecked indexing, no overflow in count parsing, catch-all in `main` |
| Security / input validation | NFR-07, FR-03, FR-05 | No file, environment, or process APIs; strict digit-only count parsing |
| Maintainability | NFR-04, NFR-05 | One unit per responsibility; pure `greeting`, `encoding`, and `run` |

## 8. Risks and open issues

| Risk / issue | Likelihood | Impact | Mitigation |
| :-- | :-- | :-- | :-- |
| CRLF on Windows from text-mode streams | M | H | `_setmode` before any output; black-box tests on Windows |
| clang-format releases format differently | M | M | Pin release line 19 in the container and natively |
| Compiler-specific warnings break `-Werror` (Apple Clang, MinGW) | M | L | Native CI on all three OSes |

## 9. Approval

| Role | Name | Date | Decision |
| :-- | :-- | :-- | :-- |
| Author | Brandon Perkins | 2026-10-13 | Submitted |
| Reviewer | Pull request review | 2026-10-13 | Pending |
