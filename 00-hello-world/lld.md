<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

**Table of Contents** _generated with [DocToc](https://github.com/thlorenz/doctoc)_

- [Low-Level Design (LLD) — Standard Terminal Hello World](#low-level-design-lld--standard-terminal-hello-world)
  - [1. Source layout](#1-source-layout)
  - [2. Data types](#2-data-types)
    - [2.1 `ParsedArguments` (cli.hpp)](#21-parsedarguments-clihpp)
    - [2.2 `GreetingRequest` (validation.hpp)](#22-greetingrequest-validationhpp)
    - [2.3 `Outcome` (app.hpp)](#23-outcome-apphpp)
    - [2.4 Error type](#24-error-type)
  - [3. Module and function specifications](#3-module-and-function-specifications)
    - [3.1 `parse_arguments`](#31-parse_arguments)
    - [3.2 `parse_count`](#32-parse_count)
    - [3.3 `validate_arguments`](#33-validate_arguments)
    - [3.4 `build_greeting_line`, `build_greeting`](#34-build_greeting_line-build_greeting)
    - [3.5 `run`](#35-run)
    - [3.6 `format_usage_error` (messages)](#36-format_usage_error-messages)
    - [3.7 `emit` (output)](#37-emit-output)
    - [3.8 `utf16_to_utf8`, `utf8_sequence_length` (encoding)](#38-utf16_to_utf8-utf8_sequence_length-encoding)
    - [3.9 Platform functions and `main`](#39-platform-functions-and-main)
  - [4. Algorithms](#4-algorithms)
  - [5. Platform-specific details](#5-platform-specific-details)
  - [6. Resource management](#6-resource-management)
  - [7. Sequence of one invocation](#7-sequence-of-one-invocation)
  - [8. Design-to-code checklist](#8-design-to-code-checklist)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Low-Level Design (LLD) — Standard Terminal Hello World

| Field                   | Value                                                 |
| :---------------------- | :---------------------------------------------------- |
| Project                 | `00-hello-world`                                      |
| Tools                   | dp-hello                                              |
| Implementation language | C++20 (CMake, CTest)                                  |
| Author                  | Brandon Perkins                                       |
| Version / date          | 1.0 — 2026-10-13                                      |
| Status                  | Approved                                              |
| Inputs                  | `hld.md`, `specs/SRS.md`, `shared/CLI-CONVENTIONS.md` |

## 1. Source layout

```text
src/
├── CMakeLists.txt              # targets: dp_hello_core (static library), dp-hello, 5 test executables
├── include/dp_hello/           # public headers of the core library (one per unit)
│   ├── usage_error.hpp         # UsageError
│   ├── cli.hpp                 # Action, OptionValue, ParsedArguments, parse_arguments
│   ├── validation.hpp          # GreetingRequest, parse_count, validate_arguments
│   ├── greeting.hpp            # build_greeting_line, build_greeting
│   ├── messages.hpp            # kHelpText, kVersionText, format_usage_error
│   ├── app.hpp                 # Outcome, run
│   ├── output.hpp              # emit
│   ├── encoding.hpp            # utf16_to_utf8, utf8_sequence_length
│   └── platform.hpp            # use_binary_standard_streams, arguments_from_native/_wide
├── lib/                        # one .cpp per header above (except usage_error)
├── app/main.cpp                # main / wmain
└── tests/                      # test_support.hpp + test_{app,cli,encoding,greeting,validation}.cpp
```

Build trees: `build/container` (container) and `build/local` (native); the executable is
`<build>/bin/dp-hello[.exe]` (`BIN_DIR`).

## 2. Data types

### 2.1 `ParsedArguments` (cli.hpp)

| Field      | Type                                  | Default | Constraint / invariant                                              | SRS                     |
| :--------- | :------------------------------------ | :------ | :------------------------------------------------------------------ | :---------------------- |
| `action`   | `Action` (`greet`, `help`, `version`) | `greet` | `help`/`version` set by the first such option; the scan stops there | FR-07, FR-08, CONV-12   |
| `name`     | `std::optional<OptionValue>`          | empty   | Last `-n`/`--name` occurrence, value unvalidated                    | FR-02, FR-06            |
| `count`    | `std::optional<OptionValue>`          | empty   | Last `-c`/`--count` occurrence, value unvalidated                   | FR-04, FR-06            |
| `operands` | `std::vector<std::string>`            | empty   | Every operand, including all tokens after `--`                      | FR-09, CONV-09, CONV-10 |

`OptionValue` holds `value` (verbatim UTF-8) and `spelling` (`-n`, `--name`, `-c`, `--count`).

### 2.2 `GreetingRequest` (validation.hpp)

| Field   | Type          | Default | Constraint / invariant | SRS           |
| :------ | :------------ | :------ | :--------------------- | :------------ |
| `name`  | `std::string` | `World` | Non-empty              | FR-01 … FR-03 |
| `count` | `int`         | 1       | 1 ≤ count ≤ 1000       | FR-04, FR-05  |

### 2.3 `Outcome` (app.hpp)

`standard_output`, `standard_error` (byte strings; at most one is non-empty) and
`exit_status` (`kExitSuccess` = 0 or `kExitUsage` = 2).

### 2.4 Error type

`UsageError{message}`; `format_usage_error` renders `dp-hello: <message>` LF
`Try 'dp-hello --help' for more information.` LF. All variants exit with status 2.

| Condition                     | `message`                                                | Produced by          |
| :---------------------------- | :------------------------------------------------------- | :------------------- |
| Unknown short option          | `invalid option -- '<letter>'` (a whole UTF-8 character) | `scan_short_options` |
| Unknown long option           | `unrecognized option '<token>'`                          | `scan_long_option`   |
| `-n`/`-c` at the end          | `option requires an argument -- '<letter>'`              | `scan_short_options` |
| `--name`/`--count` at the end | `option '--<name>' requires an argument`                 | `scan_long_option`   |
| `--help=…`/`--version=…`      | `option '--<name>' doesn't allow an argument`            | `scan_long_option`   |
| Operand                       | `extra operand '<first operand>'`                        | `validate_arguments` |
| Empty name / invalid count    | `invalid argument '<value>' for '<spelling>'`            | `validate_arguments` |

## 3. Module and function specifications

### 3.1 `parse_arguments`

- **Signature:** `ParseResult parse_arguments(std::span<const std::string> arguments)`
  (`ParseResult = std::variant<ParsedArguments, UsageError>`)
- **Responsibility:** one left-to-right scan of the arguments (class `ArgumentScanner`).
- **Preconditions:** arguments are UTF-8 and exclude the program name.
- **Postconditions:** a `ParsedArguments`, or the first malformed-option error.
- **Algorithm:** for each token: `--` → the rest are operands; `--…` → `scan_long_option`
  (split at the first `=`, exact table lookup); `-` + more → `scan_short_options` (walk UTF-8
  characters; a value option takes the rest of the token or the next argument); anything else
  → operand. Stop after `--help`/`--version`. The option table `kOptions` lists `name`,
  `count`, `help`, `version`.
- **Complexity:** O(total argument bytes) time and memory.
- **Satisfies:** FR-02, FR-06, FR-07, FR-08, FR-10, CONV-07 … CONV-13, CONV-15, CONV-22.
- **Unit tests:** `test_cli`: `no_arguments`, `name_forms`, `name_values_starting_with_dash`,
  `empty_and_inline_values`, `bundled_short_options`, `last_occurrence_wins`, `operands`,
  `help_and_version_stop_the_scan`, `unknown_options`, `missing_arguments`,
  `flags_reject_arguments`.

### 3.2 `parse_count`

- **Signature:** `std::optional<int> parse_count(std::string_view text)`
- **Algorithm:** reject empty or any non-`0`–`9` byte; skip leading zeros (all zeros → 0 →
  reject); more than 4 significant digits → reject; accumulate; accept 1 … 1000.
- **Complexity:** O(n), no overflow for any length.
- **Satisfies:** FR-04, FR-05. **Unit tests:** `test_validation`: `count_boundaries`,
  `count_leading_zeros`, `count_non_digits`, `count_huge_values_do_not_overflow`.

### 3.3 `validate_arguments`

- **Signature:** `ValidationResult validate_arguments(const ParsedArguments& arguments)`
- **Algorithm:** (1) any operand → `extra operand` with the first one; (2) a winning name that
  is empty → invalid argument; (3) a winning count that `parse_count` rejects → invalid
  argument; otherwise the request with defaults `World` / 1.
- **Satisfies:** FR-03, FR-05, FR-06, FR-09. **Unit tests:** `test_validation`: `defaults`,
  `valid_name_and_count`, `empty_name`, `invalid_count_messages`, `operand_reported_first`,
  `name_checked_before_count`.

### 3.4 `build_greeting_line`, `build_greeting`

- **Signatures:** `std::string build_greeting_line(std::string_view name)`;
  `std::string build_greeting(std::string_view name, int count)`
- **Responsibility:** pure construction of `Hello, NAME!\n`, repeated `count` times (empty for
  count ≤ 0); the result is reserved once.
- **Satisfies:** FR-01, FR-02, FR-04, NFR-05. **Unit tests:** `test_greeting`: `default_line`,
  `name_is_verbatim`, `repetition`, `maximum_count`, `lines_end_in_lf_only`.

### 3.5 `run`

- **Signature:** `Outcome run(std::span<const std::string> arguments)`
- **Algorithm:** parse → error ⇒ usage outcome; `help` ⇒ `kHelpText`; `version` ⇒
  `kVersionText`; otherwise validate → error ⇒ usage outcome, else `build_greeting`.
- **Satisfies:** FR-01 … FR-11, CONV-12, CONV-14. **Unit tests:** `test_app`: `greetings`,
  `help_and_version`, `usage_errors`, `help_text_layout`.

### 3.6 `format_usage_error` (messages)

Returns the two CONV-13/14 lines. Covered through `test_app`: `usage_errors`.

### 3.7 `emit` (output)

- **Signature:** `int emit(const Outcome& outcome)`
- **Algorithm:** `fwrite` + `fflush` stdout bytes; only if that succeeded, the stderr bytes;
  return `exit_status`. Empty strings are not written.
- **Satisfies:** FR-11, CONV-03, CONV-25. **Verification:** P01–P10.

### 3.8 `utf16_to_utf8`, `utf8_sequence_length` (encoding)

- `utf16_to_utf8`: combine valid surrogate pairs into one code point; encode an unpaired
  surrogate as itself (3 bytes); emit 1–4 UTF-8 bytes per code point.
- `utf8_sequence_length`: 2/3/4 for lead bytes `C0`–`DF`/`E0`–`EF`/`F0`–`F7`, else 1, capped at
  the remaining length.
- **Satisfies:** CONV-05. **Unit tests:** `test_encoding`: `ascii_and_bmp`, `surrogate_pairs`,
  `unpaired_surrogates`, `sequence_lengths`; `test_cli`: `unknown_options`.

### 3.9 Platform functions and `main`

`use_binary_standard_streams` (Windows `_setmode` on stdout and stderr; no-op elsewhere),
`arguments_from_native(argc, argv)` (POSIX bytes), `arguments_from_wide(argc, argv)` (Windows,
via `utf16_to_utf8`). `main`/`wmain` call `run_main`, which wraps everything in `try`/`catch`.

## 4. Algorithms

The only non-trivial algorithms are the option scan (§3.1) and the overflow-free count parse
(§3.2). The scan is the getopt model restricted to CONV-07 … CONV-12: no permutation is
needed, because operands are collected and rejected after the scan, which also makes
`extra --help` print the help (the scan reaches `--help`). Long-option lookup is an exact
string comparison against a four-entry table, so abbreviations never match (CONV-08).

## 5. Platform-specific details

| Concern      | Linux                    | macOS                    | Windows                                          | Abstraction                             |
| :----------- | :----------------------- | :----------------------- | :----------------------------------------------- | :-------------------------------------- |
| Arguments    | UTF-8 bytes in `argv`    | UTF-8 bytes in `argv`    | UTF-16 via `wmain` (`-municode`) → UTF-8         | `platform::arguments_from_*`            |
| Line endings | binary streams           | binary streams           | `_setmode(…, _O_BINARY)`                         | `platform::use_binary_standard_streams` |
| Broken pipe  | SIGPIPE ends the process | SIGPIPE ends the process | write fails → stop silently                      | `output::emit`                          |
| Compiler     | G++                      | Apple Clang              | MinGW-w64 G++, static runtime, "MinGW Makefiles" | `src/CMakeLists.txt`, `Makefile`        |

## 6. Resource management

No files, handles, processes, or locks. Memory: the argument strings plus one output string of
at most 1000 × (9 + name length) bytes, reserved once. All objects are RAII-owned
(`std::string`, `std::vector`); nothing is allocated manually.

## 7. Sequence of one invocation

`dp-hello` (P01): `main` → `arguments_from_native` returns `{}` → `run({})` →
`parse_arguments` returns `ParsedArguments{greet}` → `validate_arguments` returns
`{"World", 1}` → `build_greeting("World", 1)` = `"Hello, World!\n"` → `Outcome{…, "", 0}` →
`emit` writes 14 bytes to stdout → exit status 0.

## 8. Design-to-code checklist

| HLD component  | LLD section | Source file(s)                             | Unit-test file(s)               |
| :------------- | :---------- | :----------------------------------------- | :------------------------------ |
| cli            | §3.1        | `src/lib/cli.cpp`                          | `src/tests/test_cli.cpp`        |
| validation     | §3.2, §3.3  | `src/lib/validation.cpp`                   | `src/tests/test_validation.cpp` |
| greeting       | §3.4        | `src/lib/greeting.cpp`                     | `src/tests/test_greeting.cpp`   |
| app            | §3.5        | `src/lib/app.cpp`                          | `src/tests/test_app.cpp`        |
| messages       | §3.6        | `src/lib/messages.cpp`                     | `src/tests/test_app.cpp`        |
| output         | §3.7        | `src/lib/output.cpp`                       | black-box P01–P10               |
| encoding       | §3.8        | `src/lib/encoding.cpp`                     | `src/tests/test_encoding.cpp`   |
| platform, main | §3.9        | `src/lib/platform.cpp`, `src/app/main.cpp` | black-box P01–P10 (all OSes)    |
