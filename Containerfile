# Toolchain image for dialectic-playground/cpp: C++ (container-first; see tools/dp-run).
#
# C++20 (GCC 16 in the container); build and package tool: CMake.
# Official base image pinned by multi-architecture index digest, plus make and python3 (for the
# curriculum's black-box runner). Dependabot (docker) refreshes the digest; moving to a new
# release line is a deliberate change (keep `make toolchain` and the native CI in step).
FROM docker.io/library/gcc:16-trixie@sha256:17043c23ec2ad97fc3beca18da77275d26be5db5cd70ea0a5d73200e21a8e999

RUN apt-get update \
    && apt-get install -y --no-install-recommends make cmake python3 \
    && rm -rf /var/lib/apt/lists/*

ENV DP_IN_CONTAINER=1
