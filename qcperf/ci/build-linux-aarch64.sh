#!/usr/bin/env bash
# ============================================================================
# QcPerf - Linux ARM64 (LE) cross-compile build script
# ============================================================================
# Wraps the exact command sequence documented in README.md > Compilation
# Instructions > Linux ARM64 > Command Line, parameterized for reuse by the
# PR sanity-build workflow and the tag-triggered release workflow.
#
# Usage:
#   export AARCH64_TOOLCHAIN_PATH=/path/to/arm-gnu-toolchain
#   qcperf/ci/build-linux-aarch64.sh <BUILD_TYPE> <BUILD_SHARED> [BACKENDS]
#
#   BUILD_TYPE   - Debug | Release
#   BUILD_SHARED - ON | OFF
#   BACKENDS     - optional, semicolon-separated (default: "CPU;NPU;DUMMY",
#                  the platform-supported set for linux-aarch64)
#
# Env:
#   AARCH64_TOOLCHAIN_PATH - required, path to ARM GNU Toolchain root
#   PROJECT_VERSION        - optional, default "0.1.0.0" (matches CMake default)
# ============================================================================
set -euo pipefail

BUILD_TYPE="${1:?Usage: $0 <BUILD_TYPE Debug|Release> <BUILD_SHARED ON|OFF> [BACKENDS]}"
BUILD_SHARED="${2:?Usage: $0 <BUILD_TYPE Debug|Release> <BUILD_SHARED ON|OFF> [BACKENDS]}"
BACKENDS="${3:-CPU;NPU;DUMMY}"
PROJECT_VERSION="${PROJECT_VERSION:-0.1.0.0}"

if [[ -z "${AARCH64_TOOLCHAIN_PATH:-}" ]]; then
    echo "ERROR: AARCH64_TOOLCHAIN_PATH must be set (path to ARM GNU Toolchain root)." >&2
    exit 1
fi

SHARED_SUFFIX=""
if [[ "${BUILD_SHARED}" == "ON" ]]; then
    SHARED_SUFFIX="-shared"
fi
BUILD_DIR="build-linux-aarch64-$(echo "${BUILD_TYPE}" | tr '[:upper:]' '[:lower:]')${SHARED_SUFFIX}"

echo "==> Configuring (${BUILD_DIR})"
cmake -S qcperf -B "${BUILD_DIR}" \
    -DTARGET_ARCH=linux-aarch64 \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DProjectVersion="${PROJECT_VERSION}" \
    -DBACKENDS="${BACKENDS}" \
    -DBUILD_SHARED="${BUILD_SHARED}"

echo "==> Building (${BUILD_DIR})"
cmake --build "${BUILD_DIR}"

echo "BUILD_DIR=${BUILD_DIR}"
