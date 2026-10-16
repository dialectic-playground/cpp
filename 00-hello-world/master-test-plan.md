<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

**Table of Contents** _generated with [DocToc](https://github.com/thlorenz/doctoc)_

- [Master Test Plan (MTP) — Standard Terminal Hello World](#master-test-plan-mtp--standard-terminal-hello-world)
  - [1. Introduction and scope](#1-introduction-and-scope)
    - [1.1 Objectives](#11-objectives)
    - [1.2 In scope / out of scope](#12-in-scope--out-of-scope)
  - [2. Test items](#2-test-items)
  - [3. Test levels and approach](#3-test-levels-and-approach)
  - [4. Test design](#4-test-design)
    - [4.1 Equivalence partitions and boundaries](#41-equivalence-partitions-and-boundaries)
    - [4.2 Additional test cases (beyond the supplied suite)](#42-additional-test-cases-beyond-the-supplied-suite)
    - [4.3 Negative and robustness testing](#43-negative-and-robustness-testing)
    - [4.4 Performance and resource testing](#44-performance-and-resource-testing)
  - [5. Test environments](#5-test-environments)
  - [6. Entry and exit criteria](#6-entry-and-exit-criteria)
  - [7. Defect management](#7-defect-management)
  - [8. Schedule and deliverables](#8-schedule-and-deliverables)
  - [9. Risks and contingencies](#9-risks-and-contingencies)
  - [10. Test summary (complete at the end)](#10-test-summary-complete-at-the-end)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Master Test Plan (MTP) — Standard Terminal Hello World

| Field                   | Value                                                  |
| :---------------------- | :----------------------------------------------------- |
| Project                 | `00-hello-world`                                       |
| Tools                   | dp-hello                                               |
| Implementation language | C++20 (CMake, CTest)                                   |
| Author                  | Brandon Perkins                                        |
| Version / date          | 1.0 — 2026-10-13                                       |
| Standard                | Structure adapted from IEEE 829 / ISO/IEC/IEEE 29119-3 |

## 1. Introduction and scope

### 1.1 Objectives

"Tested enough" means: every SRS acceptance criterion (FR-01 … FR-11) is exercised by a unit
or black-box test, every option-syntax rule and every diagnostic in SRS §6 has a unit test,
the count boundaries 0/1/1000/1001 are covered, and the public and hidden suites pass on
Linux, macOS, and Windows.

### 1.2 In scope / out of scope

| In scope                                                                                   | Out of scope                                                                               |
| :----------------------------------------------------------------------------------------- | :----------------------------------------------------------------------------------------- |
| Every FR/NFR in the SRS and the applicable CONV requirements, on Linux, macOS, and Windows | Localization, terminals' rendering of non-ASCII names, file-related CONVs (not applicable) |

## 2. Test items

| Item                   | Version / location                                    |
| :--------------------- | :---------------------------------------------------- |
| dp-hello               | `00-hello-world/src/` on branch `feat/00-hello-world` |
| Public black-box suite | `tests/public/` (10 tests)                            |
| Hidden black-box suite | `tests/private/` (10 tests, run by CI)                |

## 3. Test levels and approach

| Level              | Purpose                                                        | Technique(s)                                      | Tooling                                                                                                   | Owner          |
| :----------------- | :------------------------------------------------------------- | :------------------------------------------------ | :-------------------------------------------------------------------------------------------------------- | :------------- |
| Unit               | Parsing, validation, greeting, encoding in isolation           | Equivalence partitioning, boundary-value analysis | CTest executables `test_cli`, `test_validation`, `test_greeting`, `test_encoding`                         | Developer      |
| Integration        | Argument list → `Outcome` through all logic units, without I/O | Specification-based (SRS §6 table)                | CTest executable `test_app`                                                                               | Developer      |
| System (black-box) | The specified CLI behavior, byte-exact                         | Specification-based                               | `make test` → `run-tests.py`                                                                              | Developer      |
| Acceptance         | Hidden suite passes on all OSes                                | —                                                 | CI `make check` (container) and `make check roundtrip` (native)                                           | CI             |
| Static analysis    | Warnings and formatting                                        | —                                                 | `make lint`: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror` + `clang-format --dry-run --Werror` | Developer / CI |

## 4. Test design

### 4.1 Equivalence partitions and boundaries

| Input                | Valid partitions                                         | Invalid partitions                                                        | Boundaries                                    |
| :------------------- | :------------------------------------------------------- | :------------------------------------------------------------------------ | :-------------------------------------------- |
| NAME                 | ASCII, spaces, punctuation, non-ASCII UTF-8, leading `-` | empty                                                                     | `''` (invalid), one byte (valid)              |
| COUNT                | digits with value 1–1000, leading zeros                  | empty, sign, spaces, decimal point, non-ASCII digits, 0, > 1000, huge     | 0, 1, 1000, 1001, `000`, `0001000`, 26 digits |
| Option tokens        | `-n`, `-nX`, `--name X`, `--name=X`, bundles, `--`, `-`  | unknown short/long, abbreviations, missing values, `--help=`/`--version=` | value as the last token; `--name=` (empty)    |
| Operands             | none                                                     | any word, anything after `--`                                             | first of several operands                     |
| Arguments on Windows | UTF-16 incl. surrogate pairs                             | —                                                                         | unpaired surrogates (preserved)               |

### 4.2 Additional test cases (beyond the supplied suite)

| ID   | Requirement(s) | Preconditions | Steps / command                                                  | Expected result                                         | Level       |
| :--- | :------------- | :------------ | :--------------------------------------------------------------- | :------------------------------------------------------ | :---------- |
| T-01 | FR-04, FR-05   | —             | `test_validation::count_boundaries`                              | 1 and 1000 accepted; 0, 1001 rejected                   | unit        |
| T-02 | FR-04          | —             | `test_validation::count_leading_zeros`                           | `007` → 7; `000` rejected                               | unit        |
| T-03 | FR-05, NFR-03  | —             | `test_validation::count_huge_values_do_not_overflow`             | rejected, no overflow                                   | unit        |
| T-04 | FR-03          | —             | `test_validation::empty_name`                                    | `invalid argument '' for '-n'` / `'--name'`             | unit        |
| T-05 | FR-06          | —             | `test_cli::last_occurrence_wins`                                 | last `-n` and `-c` win                                  | unit        |
| T-06 | CONV-07        | —             | `test_cli::bundled_short_options`                                | `-c2 -nKen` parsed; `-cn` gives count `n`               | unit        |
| T-07 | FR-02          | —             | `test_cli::name_values_starting_with_dash`                       | `--name=-x`, `-n-x`, `--name -x` accepted               | unit        |
| T-08 | FR-07, CONV-12 | —             | `test_cli::help_and_version_stop_the_scan`                       | `--help extra -x` → help; `-x --help` → error           | unit        |
| T-09 | FR-10          | —             | `test_cli::unknown_options`                                      | `--nam`, `--nme`, `--=x`, `-é` rejected with exact text | unit        |
| T-10 | FR-09, CONV-09 | —             | `test_app::usage_errors` (`-- -n`)                               | `extra operand '-n'`, empty stdout                      | integration |
| T-11 | CONV-05        | —             | `test_encoding::surrogate_pairs`, `unpaired_surrogates`          | correct UTF-8 / WTF-8 bytes                             | unit        |
| T-12 | CONV-25        | Linux         | start `dp-hello -c 1000` with stdout a pipe closed by the reader | ends at once, empty stderr (SIGPIPE)                    | manual      |
| T-13 | NFR-01         | CI            | P01–P10 and the hidden suite on Windows                          | LF-only bytes, `Zoë` intact                             | system      |

### 4.3 Negative and robustness testing

Every SRS §6 diagnostic has a unit case in `test_cli`, `test_validation`, or `test_app`, which
compare whole messages and confirm that stdout is empty (FR-11). Long and non-digit counts,
stray UTF-8 bytes, and unpaired surrogates are covered by T-03, T-09, and T-11. The tool reads
no files or stdin, so missing-file, permission, and binary-input cases do not apply.

### 4.4 Performance and resource testing

NFR-02: in the container, 100 runs of `dp-hello -c 1000 -n 'Ada Lovelace'` average about 1 ms
each with peak RSS about 11 MiB (measured with Python `time.perf_counter` and
`resource.getrusage(RUSAGE_CHILDREN)`), well under 100 ms and 32 MiB.

## 5. Test environments

| Environment       | OS / version                                | Toolchain                                                             | How provisioned                                     |
| :---------------- | :------------------------------------------ | :-------------------------------------------------------------------- | :-------------------------------------------------- |
| Local / container | Debian trixie (`gcc:16-trixie`)             | GCC 16, CMake 3.31, clang-format 19.1.7                               | `Containerfile`, `make check`                       |
| CI                | ubuntu-latest, macos-latest, windows-latest | G++, Apple Clang, MinGW-w64 G++ 15; CMake; clang-format 19.1.7 (PyPI) | `.github/workflows/ci.yml` (`make check roundtrip`) |

## 6. Entry and exit criteria

- **Entry:** the HLD/LLD are complete, `make all` succeeds, and `make lint` is clean.
- **Exit:** all unit tests, the public suite, and the hidden suite pass on all three OSes and
  in the container; `make lint` is clean; the RTM shows every requirement as Verified.
- **Suspension / resumption:** suspend when the build or lint fails on any platform; resume
  once it is fixed and the full CI run is green.

## 7. Defect management

Defects are GitHub issues in this repository, labelled by severity. Every fix adds a unit test
that fails before the fix and passes after it, and the full CI matrix re-runs on the pull
request.

| Severity     | Definition                                                                         |
| :----------- | :--------------------------------------------------------------------------------- |
| 1 — Critical | Crash, wrong output, or wrong exit status on a mainline path                       |
| 2 — Major    | A wrong diagnostic, or a platform-specific deviation (for example CRLF on Windows) |
| 3 — Minor    | Documentation, style, or build-tooling issues with no behavioral effect            |

## 8. Schedule and deliverables

| Milestone               | Deliverable                | Target date |
| :---------------------- | :------------------------- | :---------- |
| Design complete         | `hld.md`, `lld.md`         | 2026-10-13  |
| Implementation complete | `src/`, unit tests         | 2026-10-13  |
| Verification complete   | RTM all Verified, CI green | 2026-10-13  |

## 9. Risks and contingencies

| Risk                                                | Mitigation                                           |
| :-------------------------------------------------- | :--------------------------------------------------- |
| Windows-only failure discovered late                | Native CI on all three OSes from the first commit    |
| A compiler warns where another does not (`-Werror`) | Lint runs with each platform's compiler in CI        |
| Formatting differs between clang-format releases    | Release line 19 pinned in the container and natively |

## 10. Test summary (complete at the end)

| Metric                                | Value                                                                    |
| :------------------------------------ | :----------------------------------------------------------------------- |
| Unit tests (pass / total)             | 34 / 34 test cases in 5 CTest executables                                |
| Coverage (line / branch)              | Not measured; every function and every SRS §6 diagnostic has a unit case |
| Public black-box (pass / 10)          | 10 / 10 (container, Linux, macOS, Windows)                               |
| Hidden black-box (pass / 10, from CI) | 10 / 10 (container, Linux, macOS, Windows)                               |
| Open defects by severity              | 0 / 0 / 0                                                                |
