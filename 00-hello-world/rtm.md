# Requirements Traceability Matrix (RTM) — Standard Terminal Hello World

| Field | Value |
| :-- | :-- |
| Project | `00-hello-world` |
| Tools | dp-hello |
| Author | Brandon Perkins |
| Version / date | 1.0 — 2026-10-13 |

Unit tests are cited as `<executable>::<test case>` (files `src/tests/<executable>.cpp`, run by
CTest). Code paths are relative to `src/`. P01–P10 are the public black-box tests; the hidden
suite runs in CI on Linux, macOS, and Windows and is not cited.

## 1. User requirements to system requirements

| User requirement (URD) | Derived system requirements (SRS) |
| :-- | :-- |
| UR-01 | FR-01 |
| UR-02 | FR-02 |
| UR-03 | FR-04 |
| UR-04 | FR-07 |
| UR-05 | FR-08 |
| UR-06 | FR-03, FR-05, FR-09, FR-10, FR-11 |
| UR-07 | FR-01, SRS §3.7 |
| UR-08 | NFR-01, CONV-03 |
| UR-09 | FR-02, CONV-05 |
| UR-10 | FR-06, FR-10, CONV-07 … CONV-15 |
| UR-11 | NFR-02 |
| UR-12 | NFR-06 |

## 2. Forward traceability

| Req. ID | Summary | Design (HLD/LLD) | Code (file::function) | Verification (unit / black-box) | Status |
| :-- | :-- | :-- | :-- | :-- | :-- |
| UR-01 … UR-12 | User requirements | HLD §2 | via the derived FR/NFR rows below | via the derived FR/NFR rows below | Verified |
| FR-01 | Default greeting | HLD §3.3, LLD §3.4, §3.5 | `lib/validation.cpp::validate_arguments`, `lib/greeting.cpp::build_greeting` | `test_greeting::default_line`, `test_validation::defaults`, `test_app::greetings`, P01 | Verified |
| FR-02 | Named greeting, verbatim UTF-8 | LLD §3.1, §3.4 | `lib/cli.cpp::ArgumentScanner::scan_long_option`, `::scan_short_options`, `lib/greeting.cpp::build_greeting_line` | `test_cli::name_forms`, `test_cli::name_values_starting_with_dash`, `test_greeting::name_is_verbatim`, P02, P03, P04, P06 | Verified |
| FR-03 | Empty name rejected | LLD §3.3 | `lib/validation.cpp::validate_arguments` | `test_validation::empty_name`, `test_app::usage_errors` | Verified |
| FR-04 | Repetition 1–1000, leading zeros | LLD §3.2, §3.4 | `lib/validation.cpp::parse_count`, `lib/greeting.cpp::build_greeting` | `test_validation::count_boundaries`, `test_validation::count_leading_zeros`, `test_greeting::repetition`, `test_greeting::maximum_count`, P05, P06 | Verified |
| FR-05 | Invalid count rejected | LLD §3.2, §3.3 | `lib/validation.cpp::parse_count`, `::validate_arguments` | `test_validation::count_non_digits`, `test_validation::count_huge_values_do_not_overflow`, `test_validation::invalid_count_messages`, `test_app::usage_errors` | Verified |
| FR-06 | Last occurrence wins; validate the winner only | HLD D3, LLD §3.1 | `lib/cli.cpp::ArgumentScanner::store_value` | `test_cli::last_occurrence_wins`, `test_app::greetings` | Verified |
| FR-07 | Help text, scan order | LLD §3.1, §3.5 | `include/dp_hello/messages.hpp::kHelpText`, `lib/cli.cpp::ArgumentScanner::store_flag`, `lib/app.cpp::run` | `test_cli::help_and_version_stop_the_scan`, `test_app::help_and_version`, `test_app::help_text_layout`, P07 | Verified |
| FR-08 | Version line | LLD §3.5 | `include/dp_hello/messages.hpp::kVersionText`, `lib/app.cpp::run` | `test_app::help_and_version`, P08 | Verified |
| FR-09 | Operands rejected | LLD §3.3 | `lib/cli.cpp::ArgumentScanner::scan`, `lib/validation.cpp::validate_arguments` | `test_cli::operands`, `test_validation::operand_reported_first`, `test_app::usage_errors`, P10 | Verified |
| FR-10 | Unknown/malformed options, no abbreviations | LLD §2.4, §3.1 | `lib/cli.cpp::ArgumentScanner::scan_long_option`, `::scan_short_options`, `find_long_option` | `test_cli::unknown_options`, `test_cli::missing_arguments`, `test_cli::flags_reject_arguments`, `test_app::usage_errors`, P09 | Verified |
| FR-11 | Nothing on stdout on error | HLD D1, D4, LLD §3.5, §3.7 | `lib/app.cpp::run`, `lib/output.cpp::emit` | `test_app::usage_errors` (empty stdout), P09, P10 | Verified |
| NFR-01 | Byte-identical, LF-only on all OSes | HLD §5, LLD §5 | `lib/platform.cpp::use_binary_standard_streams`, `lib/greeting.cpp::build_greeting_line` | `test_greeting::lines_end_in_lf_only`, P01–P10 on Linux, macOS, Windows (CI) | Verified |
| NFR-02 | < 100 ms, < 32 MiB | HLD §7 | native executable, `lib/greeting.cpp::build_greeting` (one reservation) | Measurement: master test plan §4.4 | Verified |
| NFR-03 | No crash for any arguments | HLD §6 | `app/main.cpp::run_main`, `lib/validation.cpp::parse_count` | `test_validation::count_huge_values_do_not_overflow`, `test_cli::unknown_options`, P09, P10 | Verified |
| NFR-04 | Separate parsing, validation, greeting, output units | HLD §3.2, LLD §1 | `lib/cli.cpp`, `lib/validation.cpp`, `lib/greeting.cpp`, `lib/output.cpp` | Design review (LLD §8) | Verified |
| NFR-05 | Pure, unit-tested greeting construction | LLD §3.4 | `lib/greeting.cpp::build_greeting_line`, `::build_greeting` | `test_greeting::*` | Verified |
| NFR-06 | All BASE-MAKEFILE targets work | LLD §1 | `Makefile` (`DP_*` hooks), `src/CMakeLists.txt` | CI: `make check roundtrip` on 3 OSes, `make check` in the container | Verified |
| NFR-07 | No files, environment, or subprocesses | HLD §3.1 | whole `src/` (inspection: only `fwrite`/`fflush`/`_setmode`) | Inspection | Verified |
| CONV-01 | Canonical name in all output | LLD §3.6 | `include/dp_hello/messages.hpp::kToolName`, `kHelpText`, `kVersionText` | `test_app::usage_errors`, P07, P08, P09 | Verified |
| CONV-02 | Version format | LLD §3.5 | `include/dp_hello/messages.hpp::kVersionText` | `test_app::help_and_version`, P08 | Verified |
| CONV-03 | LF only, binary streams on Windows | HLD §5 | `lib/platform.cpp::use_binary_standard_streams`, `lib/output.cpp::emit` | `test_greeting::lines_end_in_lf_only`, `test_app::help_text_layout`, P01–P10 (Windows CI) | Verified |
| CONV-05 | UTF-8 arguments (Windows: UTF-16 → UTF-8) | HLD D5, D6, LLD §3.8 | `lib/encoding.cpp::utf16_to_utf8`, `lib/platform.cpp::arguments_from_wide`, `app/main.cpp::wmain` | `test_encoding::*`, `test_validation::valid_name_and_count`, `test_greeting::name_is_verbatim` | Verified |
| CONV-06 | No locale dependence, plain output | HLD §5 | no locale or terminal APIs anywhere in `src/` | Inspection, P01–P10 | Verified |
| CONV-07 | Short options, attached values, bundling | LLD §3.1 | `lib/cli.cpp::ArgumentScanner::scan_short_options` | `test_cli::name_forms`, `test_cli::bundled_short_options`, P03, P06 | Verified |
| CONV-08 | Long options, `=value`, no abbreviation | LLD §3.1 | `lib/cli.cpp::ArgumentScanner::scan_long_option`, `find_long_option` | `test_cli::name_forms`, `test_cli::empty_and_inline_values`, `test_cli::unknown_options`, P02, P04, P05 | Verified |
| CONV-09 | `--` terminator | LLD §3.1 | `lib/cli.cpp::ArgumentScanner::take_remaining_as_operands` | `test_cli::operands`, `test_app::usage_errors` | Verified |
| CONV-10 | Operands, `-` is an operand | LLD §3.1 | `lib/cli.cpp::ArgumentScanner::scan` | `test_cli::operands`, P10 | Verified |
| CONV-11 | Repetition: last wins | LLD §3.1 | `lib/cli.cpp::ArgumentScanner::store_value` | `test_cli::last_occurrence_wins` | Verified |
| CONV-12 | Scan order; errors before output | HLD D3, D4 | `lib/cli.cpp::ArgumentScanner::scan`, `lib/app.cpp::run` | `test_cli::help_and_version_stop_the_scan`, `test_app::help_and_version`, `test_app::usage_errors` | Verified |
| CONV-13 | Exact usage-error messages | LLD §2.4 | `lib/cli.cpp::make_error`, `lib/validation.cpp::invalid_argument` | `test_cli::unknown_options`, `test_cli::missing_arguments`, `test_cli::flags_reject_arguments`, `test_validation::invalid_count_messages`, P09, P10 | Verified |
| CONV-14 | "Try …" line, exit 2 | LLD §2.4, §3.6 | `lib/messages.cpp::format_usage_error`, `lib/app.cpp::usage_failure` | `test_app::usage_errors`, P09, P10 | Verified |
| CONV-15 | Values verbatim, empty as `''` | LLD §2.4 | `lib/validation.cpp::invalid_argument` | `test_validation::empty_name`, `test_validation::invalid_count_messages` | Verified |
| CONV-18 | Exit statuses 0 / 2 | LLD §2.3 | `include/dp_hello/app.hpp::kExitSuccess`, `kExitUsage` | `test_app::*`, P01–P10 | Verified |
| CONV-22 | `--help`, `--version` take no argument | LLD §3.1 | `lib/cli.cpp::ArgumentScanner::scan_long_option` | `test_cli::flags_reject_arguments` | Verified |
| CONV-23 | Help text verbatim | LLD §3.6 | `include/dp_hello/messages.hpp::kHelpText` | `test_app::help_text_layout`, P07 | Verified |
| CONV-24 | No crash or stack trace | HLD §6 | `app/main.cpp::run_main` | `test_validation::count_huge_values_do_not_overflow`, P09, P10 | Verified |
| CONV-25 | EPIPE: stop silently | HLD §6, LLD §3.7 | `lib/output.cpp::emit` | Master test plan T-12 (manual) | Verified |
| CONV-26 | stdin not read | HLD §3.1 | no stdin access in `src/` | Inspection | Verified |
| CONV-27 … CONV-30 | Harness conditions | — | native executable in `BIN_DIR` | P01–P10 | Verified |

CONV-04, CONV-16, CONV-17, and CONV-19 … CONV-21 do not apply: `dp-hello` copies no input and
handles no files or paths.

## 3. Backward traceability (test → requirement)

| Test | Type | Requirement(s) verified |
| :-- | :-- | :-- |
| P01 | black-box, public | FR-01, CONV-03 |
| P02, P03, P04 | black-box, public | FR-02, CONV-07, CONV-08 |
| P05, P06 | black-box, public | FR-04, CONV-07 |
| P07 | black-box, public | FR-07, CONV-23 |
| P08 | black-box, public | FR-08, CONV-02 |
| P09 | black-box, public | FR-10, FR-11, CONV-13, CONV-14 |
| P10 | black-box, public | FR-09, FR-11, CONV-10, CONV-13 |
| `test_cli::*` (11 cases) | unit | FR-02, FR-06, FR-07, FR-08, FR-10, CONV-07 … CONV-13, CONV-22 |
| `test_validation::*` (10 cases) | unit | FR-03, FR-04, FR-05, FR-06, FR-09, CONV-15 |
| `test_greeting::*` (5 cases) | unit | FR-01, FR-02, FR-04, NFR-01, NFR-05 |
| `test_encoding::*` (4 cases) | unit | CONV-05 |
| `test_app::*` (4 cases) | unit (integration) | FR-01 … FR-11, CONV-12, CONV-14, CONV-23 |

## 4. Coverage summary

| Category | Total | Designed | Implemented | Verified |
| :-- | :-- | :-- | :-- | :-- |
| User requirements (UR) | 12 | 12 | 12 | 12 |
| Functional (FR) | 11 | 11 | 11 | 11 |
| Non-functional (NFR) | 7 | 7 | 7 | 7 |
| Conventions (CONV, applicable) | 24 | 24 | 24 | 24 |

## 5. Gaps and justifications
None. NFR-02, NFR-04, NFR-07, CONV-06, and CONV-26 are verified by measurement or inspection,
as SRS §7 prescribes; CONV-25 is verified manually (master test plan T-12) because the
black-box harness cannot close a pipe early.
