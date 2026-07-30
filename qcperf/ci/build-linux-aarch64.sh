#!/usr/bin/env bash
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted (subject to the limitations in the
# disclaimer below) provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#     * Neither the name of Qualcomm Technologies, Inc. nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
# NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
# GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
# HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
# BACKENDS="${3:-CPU;NPU;DUMMY}"
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
    -DBUILD_SHARED="${BUILD_SHARED}"

echo "==> Building (${BUILD_DIR})"
cmake --build "${BUILD_DIR}"

echo "BUILD_DIR=${BUILD_DIR}"
