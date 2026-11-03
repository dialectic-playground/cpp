# Toolchain image for dialectic-playground/cpp: C++ (container-first; see tools/dp-run).
#
# C++20 (GCC 16 in the container); build and package tool: CMake.
# Official base image pinned by multi-architecture index digest, plus make, python3 (for the
# curriculum's black-box runner), and clang-format 19 (Debian trixie; `make lint` / `make format`).
# Dependabot (docker) refreshes the digest; moving to a new release line is a deliberate change
# (keep `make toolchain` and the native CI in step).
FROM docker.io/library/gcc:16-trixie@sha256:07280eb2e744f1b7a2d8d4a3894c089ea07266c109758afc388a634bd098e4eb

# Debian keeps only the current version of each package, so apt versions cannot be pinned
# reproducibly; the base image is pinned by digest instead.
# hadolint ignore=DL3008
RUN apt-get update \
    && apt-get install -y --no-install-recommends make cmake python3 clang-format \
    && rm -rf /var/lib/apt/lists/*

ENV DP_IN_CONTAINER=1
