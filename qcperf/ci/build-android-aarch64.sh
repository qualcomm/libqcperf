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
# QcPerf - Android ARM64 cross-compile build script
# ============================================================================
# Wraps the exact command sequence documented in README.md > Compilation
# Instructions > Android ARM64 > Command Line, parameterized for reuse by the
# PR sanity-build workflow and the tag-triggered release workflow.
#
# Usage:
#   export ANDROID_NDK_PATH=/path/to/android-ndk
#   qcperf/ci/build-android-aarch64.sh <BUILD_TYPE> <BUILD_SHARED> [BACKENDS]
#
#   BUILD_TYPE   - Debug | Release
#   BUILD_SHARED - ON | OFF
#   BACKENDS     - optional, semicolon-separated. Omit to let CMake enable all
#                  backends supported on android-aarch64 (its default behavior).
#
# Env:
#   ANDROID_NDK_PATH - required, path to Android NDK root
#   PROJECT_VERSION  - optional, default "0.1.0.0" (matches CMake default)
#   ANDROID_API      - optional, default 29
# ============================================================================
set -euo pipefail

BUILD_TYPE="${1:?Usage: $0 <BUILD_TYPE Debug|Release> <BUILD_SHARED ON|OFF> [BACKENDS]}"
BUILD_SHARED="${2:?Usage: $0 <BUILD_TYPE Debug|Release> <BUILD_SHARED ON|OFF> [BACKENDS]}"
BACKENDS="${3:-}"
PROJECT_VERSION="${PROJECT_VERSION:-0.1.0.0}"
ANDROID_API="${ANDROID_API:-29}"

if [[ -z "${ANDROID_NDK_PATH:-}" ]]; then
    echo "ERROR: ANDROID_NDK_PATH must be set (path to Android NDK root)." >&2
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
NDK_TOOLCHAIN="${ANDROID_NDK_PATH}/toolchains/llvm/prebuilt/linux-x86_64"
CC_BIN="${NDK_TOOLCHAIN}/bin/aarch64-linux-android${ANDROID_API}-clang"
AR_BIN="${NDK_TOOLCHAIN}/bin/llvm-ar"
RANLIB_BIN="${NDK_TOOLCHAIN}/bin/llvm-ranlib"

echo "==> Building fastrpc (libcdsprpc, required by the NPU backend)"
"${SCRIPT_DIR}/build-fastrpc.sh" "${CC_BIN}" "${AR_BIN}" "${RANLIB_BIN}"

BUILD_DIR="build-android-aarch64-$(echo "${BUILD_TYPE}" | tr '[:upper:]' '[:lower:]')"
if [[ "${BUILD_SHARED}" == "ON" ]]; then
    BUILD_DIR="${BUILD_DIR}-shared"
fi

CMAKE_ARGS=(
    -S qcperf -B "${BUILD_DIR}"
    -DCMAKE_TOOLCHAIN_FILE="${ANDROID_NDK_PATH}/build/cmake/android.toolchain.cmake"
    -DANDROID_ABI=arm64-v8a
    -DANDROID_PLATFORM="android-${ANDROID_API}"
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
    -DProjectVersion="${PROJECT_VERSION}"
    -DBUILD_SHARED="${BUILD_SHARED}"
)
if [[ -n "${BACKENDS}" ]]; then
    CMAKE_ARGS+=(-DBACKENDS="${BACKENDS}")
fi

echo "==> Configuring (${BUILD_DIR})"
cmake "${CMAKE_ARGS[@]}"

echo "==> Building (${BUILD_DIR})"
cmake --build "${BUILD_DIR}"

echo "BUILD_DIR=${BUILD_DIR}"
