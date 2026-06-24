# ============================================================================
# QcPerf - Qualcomm Performance Profiler Library
# Main CMakeLists.txt file
# ============================================================================

# Minimum CMake version required for modern CMake features
# Version 3.15+ provides improved target property handling and generator expressions
cmake_minimum_required(VERSION 3.15)

# ============================================================================
# Select Toolchain based on TARGET_ARCH
# ============================================================================

# ============================================================================
# Library type option
# ============================================================================
# Build QcPerfCore as STATIC by default.
# Pass -DBUILD_SHARED=ON to build a shared library (.so / .dll) instead.
option(BUILD_SHARED "Build QcPerfCore as a shared library (.so/.dll)" OFF)

if(NOT CMAKE_TOOLCHAIN_FILE AND DEFINED TARGET_ARCH)
    if(TARGET_ARCH STREQUAL "linux-aarch64")
        # Linux AArch64 cross-compilation — set the GNU cross-toolchain file.
        set(CMAKE_TOOLCHAIN_FILE
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchains/aarch64-linux-gnu.cmake"
            CACHE FILEPATH "Toolchain file (auto-selected for TARGET_ARCH=${TARGET_ARCH})")
        message(STATUS "Selected toolchain for TARGET_ARCH=${TARGET_ARCH}: ${CMAKE_TOOLCHAIN_FILE}")
    endif()
endif()

# ============================================================================
# Version information
# ============================================================================
# Enable version information generation
# This creates a header with version details, build timestamp, etc.
add_subdirectory(version_info)

# ============================================================================
# Project declaration
# ============================================================================
# - VERSION: Semantic versioning (major.minor.patch)
# - DESCRIPTION: Brief description of the project
# - LANGUAGES: Programming languages used (C only for this project)
project(QcPerf
    VERSION "${QCPERF_VERSION}"
    DESCRIPTION "Qualcomm Performance Profiler Library v${QCPERF_VERSION}"
    LANGUAGES C CXX
)

# Set Binary root directory for all output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ============================================================================
# Build configuration
# ============================================================================
# Include the combined build configuration from cmake/BuildConfig.cmake
# This centralizes platform detection, compiler flags, and other build settings
include(cmake/BuildConfig.cmake)

if(BUILD_SHARED)
    set(BUILD_SHARED_TYPE "SHARED")
    add_compile_definitions(QCPERF_SHARED_LIBRARY)
else()
    add_compile_definitions(QCPERF_STATIC_LIBRARY)
    set(BUILD_SHARED_TYPE "STATIC")
endif()


# ============================================================================
# Component subdirectories
# ============================================================================
# Add each component in dependency order:
# - core: Main library functionality
# - backends: Performance monitoring backend implementations
# - utils: Utility libraries used by core and backends
# - core-test: Test application for the core library
add_subdirectory(core)
add_subdirectory(backends)
add_subdirectory(utils)
add_subdirectory(test_app)
add_subdirectory(npu_logger)

# ============================================================================
# Configuration summary
# ============================================================================
# Print a summary of the build configuration for user verification

# Build the list of enabled backends for display
set(_ENABLED_BACKENDS_LIST "")
foreach(_BACKEND ${QCPERF_PLATFORM_SUPPORTED_BACKENDS})
    if(DEFINED QCPERF_BACKEND_OS_PREFIX_${_BACKEND})
        set(_OS_PREFIX ${QCPERF_BACKEND_OS_PREFIX_${_BACKEND}})
        set(_COMPILE_FLAG QCPERF_ENABLED_${_OS_PREFIX}_${_BACKEND})
    else()
        set(_COMPILE_FLAG QCPERF_ENABLED_${_BACKEND})
    endif()
    if(${_COMPILE_FLAG})
        list(APPEND _ENABLED_BACKENDS_LIST ${_COMPILE_FLAG})
    endif()
endforeach()
string(REPLACE ";" ", " _ENABLED_BACKENDS_STR "${_ENABLED_BACKENDS_LIST}")

message(STATUS "")
message(STATUS "QcPerf Configuration Summary:")
message(STATUS "  Version: ${QCPERF_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  CXX Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "  System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Processor: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Library type: ${BUILD_SHARED_TYPE}")
message(STATUS "  Enabled backends: ${_ENABLED_BACKENDS_STR}")
message(STATUS "")
