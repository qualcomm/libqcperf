# libqcperf

A lightweight, open-source performance profiling library for Qualcomm chipsets.

## Table of Contents

- [libqcperf](#libqcperf)
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
  - [Framework Features](#framework-features)
    - [Core Capabilities](#core-capabilities)
    - [Available Backends](#available-backends)
    - [Technical Foundation](#technical-foundation)
  - [Build System](#build-system)
  - [Compilation Instructions](#compilation-instructions)
    - [Android ARM64](#android-arm64-compilation)
    - [Linux ARM64](#linux-arm64-compilation)
      - [Command Line](#command-line)
      - [CMake Presets](#cmake-presets)
    - [Windows ARM64](#windows-arm64-compilation)
  - [Running QcPerfCoreTest](#running-qcperfcoretest)
    - [Usage](#usage)
    - [Android ARM64](#android-arm64)
  - [Design Diagrams](#design-diagrams)
    - [Sequence Diagram](#sequence-diagram)
    - [Flow Diagram](#flow-diagram)
    - [Backend Architecture](#backend-architecture)
  - [Documentation for backend developers](#documentation-for-backend-developers)
  - [Contributing](#contributing)
  - [Security](#security)
  - [License](#license)

## Introduction

libqcperf is a performance profiling library designed for Qualcomm chipsets across different SoCs and subsystems. It provides a unified framework for monitoring and analyzing system performance metrics in real-time, enabling developers to optimize their applications for Qualcomm platforms.

The library is designed with modularity and extensibility in mind, featuring a core API that interfaces with multiple specialized backends. Each backend focuses on specific performance aspects such as thermal monitoring, memory bandwidth, or power consumption. This architecture allows developers to collect precisely the metrics they need without unnecessary overhead.

Originally inspired by the Qualcomm Profiler tool, libqcperf is now available as an open-source project, empowering the developer community to build from source, customize functionality, and extend capabilities through new backends or integrations with other tools.

## Framework Features

### Core Capabilities
- Modular backend architecture for different monitoring needs
- Configurable sampling and streaming rates
- Dynamic capability discovery
- Cross-platform support (Windows and Linux)
- Asynchronous data collection via background threads
- Non-blocking callback-based data delivery

### Available Backends

| Backend | OS / Platform | `-DBACKENDS` Flag | Compile Definition | Description | Metrics & Configuration Reference |
|---------|--------------|-------------------|--------------------|-------------|-----------------------------------|
| **Thermal Backend** | Windows on Snapdragon (WoS) | `THERMAL` | `QCPERF_ENABLED_WOS_THERMAL` | Monitors temperature and passive cooling metrics across 22 thermal zones | [wos_thermal_info.h](qcperf/backends/wos-thermal/inc/wos_thermal_info.h) |
| **Power Backend** | Windows on Snapdragon (WoS) | `POWER` | `QCPERF_ENABLED_WOS_POWER` | Measures power consumption across various system components | [wos_power_backend_info.h](qcperf/backends/wos-power-backend/inc/wos_power_backend_info.h) |
| **Linux CPU Backend** | Qcom Linux | `CPU` | `QCPERF_ENABLED_QCOM_LINUX_CPU` | Monitors CPU load, frequency, effective utilization, steal time, and DCVS frequency limit per core (up to 18 cores) | [qcom_linux_cpu_info.h](qcperf/backends/qcom-linux-cpu/inc/qcom_linux_cpu_info.h) |
| **DSP NPU Backend** | Qcom Linux | `NPU` | `QCPERF_ENABLED_QCOM_LINUX_NPU` | Monitors Neural Processing Unit (NPU) performance metrics including Q6 utilization, Q6 clock frequency, HVX utilization, and HMX utilization | [qcom_dsp_npu_info.h](qcperf/backends/qcom-dsp-npu/inc/qcom_dsp_npu_info.h) |
| **Dummy Backend** | All | `DUMMY` | `QCPERF_ENABLED_DUMMY` | Reference implementation for testing and development | [dummy_info.h](qcperf/backends/dummy/inc/dummy_info.h) |

> **Note:** Each backend's info.h file contains detailed information about supported capability, metrics, sampling rates, and streaming rates.
>

### Technical Foundation
- WoS backends rely on publicly available Windows driver APIs (ETW, IOCTL)
- Linux backends rely on standard Linux kernel interfaces (sysfs, procfs)
- Comprehensive error handling and resource management
- Thread-safe operations for multi-threaded applications

## Build System

- **CMake**
- **Compiler / Toolchain**
  - Windows on Snapdragon - Visual Studio 2022
  - Linux ARM64 - ARM GNU Toolchain (`aarch64-none-linux-gnu-gcc`)
  - Android ARM64 - Android NDK (r24+, `arm64-v8a`, API 29+)

## Compilation Instructions

### Android ARM64 Compilation

To cross-compile the library for Android on ARM64 platforms, you need the [Android NDK](https://developer.android.com/ndk/downloads) (NDK r24 or later recommended).

**Supported backends:** `DUMMY`, `CPU`, `NPU`

The NPU backend depends on `libcdsprpc.so` from the [fastrpc](https://github.com/qualcomm/fastrpc) submodule.

```bash
# Set NDK path
NDK=/path/to/android-ndk           # e.g. /opt/android-ndk/24.0.8215888
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
API=29
export CC=$TOOLCHAIN/bin/aarch64-linux-android${API}-clang
export AR=$TOOLCHAIN/bin/llvm-ar
export RANLIB=$TOOLCHAIN/bin/llvm-ranlib

# Build fastrpc (libcdsprpc.so)
git submodule update --init qcperf/third-party/fastrpc
cd qcperf/third-party/fastrpc && bash autogen.sh && ./configure --host=aarch64-linux-android && cd -
make -C qcperf/third-party/fastrpc/src libcdsprpc.la

# Build libqcperf
cmake -S qcperf -B build-android-aarch64-release \
    -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-29 \
    -DCMAKE_BUILD_TYPE=Release \
    -DProjectVersion="0.1.0.0" \
    -DBACKENDS="CPU;NPU;DUMMY" \
    -DBUILD_SHARED=OFF
cmake --build build-android-aarch64-release
```

> **Note:** `-DBUILD_SHARED=ON` builds `libqcperfCore.so` instead of the default static `libqcperfCore.a` (see the `-DBUILD_SHARED` flag description in the Linux ARM64 section below).

> **Note:** When running on the device, if `libcdsprpc.so` is not found at runtime, set:
> ```bash
> export LD_LIBRARY_PATH=/vendor/lib64
> ```

### Linux ARM64 Compilation

To cross-compile the library for Linux on ARM64 platforms, you need the [ARM GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (`arm-gnu-toolchain-**.*.rel1-x86_64-aarch64-none-linux-gnu`).

The NPU backend depends on `libcdsprpc.so` from the [fastrpc](https://github.com/qualcomm/fastrpc) submodule.

#### Command Line

```bash
# Set the toolchain path (adjust to your installation directory)
export AARCH64_TOOLCHAIN_PATH=/path/to/arm-gnu-toolchain
export CC=$AARCH64_TOOLCHAIN_PATH/bin/aarch64-none-linux-gnu-gcc
export AR=$AARCH64_TOOLCHAIN_PATH/bin/aarch64-none-linux-gnu-ar
export RANLIB=$AARCH64_TOOLCHAIN_PATH/bin/aarch64-none-linux-gnu-ranlib

# Build fastrpc (libcdsprpc.so)
git submodule update --init qcperf/third-party/fastrpc
cd qcperf/third-party/fastrpc && bash autogen.sh && ./configure --host=aarch64-linux-android && cd -
make -C qcperf/third-party/fastrpc/src libcdsprpc.la

# Build libqcperf
cmake -S qcperf -B build-linux-aarch64-release \
    -DTARGET_ARCH=linux-aarch64 \
    -DCMAKE_BUILD_TYPE=Release \
    -DProjectVersion="0.1.0.0" \
    -DBACKENDS="CPU;NPU;DUMMY" \
    -DBUILD_SHARED=OFF
cmake --build build-linux-aarch64-release
```

| Flag | Value | Description |
|------|-------|-------------|
| `-S qcperf` | `qcperf` | Source directory containing the top-level `CMakeLists.txt` |
| `-B build-linux-aarch64-release` | `build-linux-aarch64-release` | Output directory for generated build files and compiled artifacts |
| `-DTARGET_ARCH` | `linux-aarch64` | Selects the cross-compilation toolchain for Linux ARM64; requires `AARCH64_TOOLCHAIN_PATH` to be set |
| `-DCMAKE_BUILD_TYPE` | `Release` / `Debug` | Build type: `Release` produces an optimized binary; `Debug` includes debug symbols |
| `-DProjectVersion` | `"0.1.0.0"` | Sets the library version string embedded in the build |
| `-DBACKENDS` | `"CPU;NPU;DUMMY"` | Semicolon-separated list of backends to compile (case-insensitive); omit this flag to compile all backends supported on the target platform |
| `-DBUILD_SHARED` | `OFF` / `ON` | `OFF` builds a static library (`.a`); `ON` builds a shared library (`.so`) |

> **Note:** Only backends supported on the target platform are compiled even if listed explicitly (e.g., specifying `power` on Linux ARM64 will be silently skipped with a warning).

#### CMake Presets

The build is configured via CMake presets defined in `qcperf/CMakePresets.json` (shared base) and `qcperf/CMakeUserPresets.json` (user-local overrides).

**Toolchain path — priority order:**

1. Environment variable `AARCH64_TOOLCHAIN_PATH` (takes precedence if set)
2. Value defined in the `linux-aarch64-user-base` hidden preset in `qcperf/CMakeUserPresets.json` (fallback default)

To change the toolchain path or project version, edit **only** the `linux-aarch64-user-base` preset in `qcperf/CMakeUserPresets.json` — all other presets inherit from it automatically.

| Preset | Build Type | Library Type | Output Directory |
|--------|-----------|--------------|-----------------|
| `linux-aarch64-debug` | Debug | Static | `build-linux-aarch64-debug/` |
| `linux-aarch64-release` | Release | Static | `build-linux-aarch64-release/` |
| `linux-aarch64-debug-shared` | Debug | Shared (`.so`) | `build-linux-aarch64-debug-shared/` |
| `linux-aarch64-release-shared` | Release | Shared (`.so`) | `build-linux-aarch64-release-shared/` |

e.g. for a debug build:

```bash
# Configure
cd qcperf
cmake --preset linux-aarch64-debug

# Compile the project
cmake --build --preset linux-aarch64-debug
```

e.g. for a release shared library build:

```bash
# Configure
cd qcperf
cmake --preset linux-aarch64-release-shared

# Compile the project
cmake --build --preset linux-aarch64-release-shared
```

### Windows ARM64 Compilation

To build the library for Windows on ARM64 platforms:

```bash
# Clone the repository
git clone https://github.com/qualcomm/libqcperf.git
cd libqcperf

# Clone third-party dependencies
git submodule update --init --recursive

# Sample test app path
qcperf\test_app

# Compilation for Windows ARM64
## Generate build files
cmake -B <build_dir_path> -G "Visual Studio 17 2022" -A ARM64 -DProjectVersion="0.1.0.0"

## Compile the project
cmake --build <build_dir_path> --config Release
```

## Running QcPerfCoreTest

`QcPerfCoreTest` is a small C test application that exercises the full library flow (initialize → connect to a backend → query capabilities → stream metrics → disconnect → deinitialize). It is built automatically alongside the library and placed at the root of the build directory (e.g. `build-android-aarch64-release/QcPerfCoreTest`).

### Usage

```bash
QcPerfCoreTest <backend_id> <sampling_rate_ms> <streaming_rate_ms> <verbose>
```

| Argument | Description |
|----------|-------------|
| `backend_id` | Numeric backend to connect to: `0` = DUMMY, `1` = CPU, `2` = NPU |
| `sampling_rate_ms` | Sampling rate in milliseconds |
| `streaming_rate_ms` | Streaming (callback) rate in milliseconds |
| `verbose` | `0` = off, `1` = on |

> **Note:** The `backend_id` must correspond to a backend compiled into the build via `-DBACKENDS`. Connecting to a backend that was not compiled in returns an invalid-backend error.

### Android ARM64

Push the binary to `/vendor/bin` and run it on the device via `adb`.

```bash
# Elevate and make /vendor writable
adb root
adb remount

# Push the test binary to /vendor/bin
adb push build-android-aarch64-release/QcPerfCoreTest /vendor/bin/
adb shell chmod +x /vendor/bin/QcPerfCoreTest

# Run — example: DUMMY backend, 100 ms sampling, 1000 ms streaming, verbose on
adb shell /vendor/bin/QcPerfCoreTest 0 100 1000 1

# Run the NPU backend
adb shell /vendor/bin/QcPerfCoreTest 2 100 1000 1
```

## Design Diagrams

### Sequence Diagram

- [QcPerf Sequence Diagram](./assets/libqcperf_sequence.mmd)

### Flow Diagram

- [QcPerf Flow Diagram](./assets/libqcperf_flow.mmd)

### Backend Architecture

- [QcPerf Backend Architecture](./assets/libqcperf_backend_architecture.mmd)

## Documentation for backend developers

For detailed information about backend development, coding standards, and extending the library, please refer to the [DEVELOPMENT-GUIDE.md](./DEVELOPMENT-GUIDE.md) file in the root of the repository.

## Contributing

We welcome contributions to libqcperf! Please read our [CONTRIBUTING.md](CONTRIBUTING.md) file for details on our code of conduct and the process for submitting pull requests.

## Security

For information on reporting security vulnerabilities, please see our [SECURITY.md](SECURITY.md) file.

## License

This project is licensed under the BSD 3-Clause License. See the [LICENSE.txt](LICENSE.txt) file for full details.

Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
