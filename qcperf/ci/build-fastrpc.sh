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
# QcPerf - fastrpc (libcdsprpc) cross-build for CI
# ============================================================================
# Builds the fastrpc submodule (qcperf/third-party/fastrpc) from source so
# that libcdsprpc.so exists at third-party/fastrpc/src/.libs, satisfying the
# find_library() lookup in qcperf/backends/qcom-dsp/CMakeLists.txt. Without
# this, the NPU backend (which links against libcdsprpc) cannot configure on
# hosted runners, which ship no Qualcomm BSP.
#
# This only needs to produce a compile-time link target; the real
# libcdsprpc.so used at runtime on-device comes from the Qualcomm BSP and is
# unrelated to this build.
#
# Requires (installed by this script via apt, needs sudo):
#   autotools-dev automake autoconf libtool pkg-config
#   gcc-aarch64-linux-gnu g++-aarch64-linux-gnu binutils-aarch64-linux-gnu
#   libyaml-dev libyaml-0-2:arm64 libyaml-dev:arm64 libbsd-dev:arm64
#
# Usage:
#   qcperf/ci/build-fastrpc.sh
# ============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FASTRPC_DIR="${SCRIPT_DIR}/../third-party/fastrpc"

if [[ ! -f "${FASTRPC_DIR}/configure.ac" ]]; then
    echo "ERROR: ${FASTRPC_DIR} does not look like the fastrpc submodule (missing configure.ac)." >&2
    echo "       Did you check out submodules? (actions/checkout with submodules: true)" >&2
    exit 1
fi

if [[ -f "${FASTRPC_DIR}/src/.libs/libcdsprpc.so" ]]; then
    echo "==> fastrpc already built, skipping"
    exit 0
fi

echo "==> Enabling arm64 multiarch and installing fastrpc build dependencies"
sudo dpkg --add-architecture arm64
sudo apt-get update -y
sudo apt-get install -y --no-install-recommends \
    automake autoconf libtool pkg-config \
    gcc-aarch64-linux-gnu g++-aarch64-linux-gnu binutils-aarch64-linux-gnu \
    libyaml-dev \
    libyaml-0-2:arm64 libyaml-dev:arm64 \
    libbsd-dev:arm64

echo "==> Building fastrpc (${FASTRPC_DIR})"
(
    cd "${FASTRPC_DIR}"
    export CC=aarch64-linux-gnu-gcc
    export CXX=aarch64-linux-gnu-g++
    export AS=aarch64-linux-gnu-as
    export LD=aarch64-linux-gnu-ld
    export RANLIB=aarch64-linux-gnu-ranlib
    export STRIP=aarch64-linux-gnu-strip
    export PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig
    export CFLAGS="-O2"
    export CXXFLAGS="-O2"
    ./gitcompile --host=aarch64-linux-gnu
)

if [[ ! -f "${FASTRPC_DIR}/src/.libs/libcdsprpc.so" ]]; then
    echo "ERROR: fastrpc build did not produce src/.libs/libcdsprpc.so" >&2
    exit 1
fi

echo "==> fastrpc build complete: ${FASTRPC_DIR}/src/.libs/libcdsprpc.so"
