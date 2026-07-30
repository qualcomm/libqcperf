# ============================================================================
# QcPerf - Linux Platform Configuration
# ============================================================================
# This file contains Linux-specific build configurations for the QcPerf library.
# It sets compiler flags, preprocessor definitions, and library dependencies
# specific to Linux platforms, with special handling for different architectures
# (ARM64, x86_64) and compilers (GCC, Clang).
#
# This file is included by BuildConfig.cmake when building on Linux platforms.
# ============================================================================

# ============================================================================
# Linux Platform Definitions
# ============================================================================
# Enable Linux-specific preprocessor definitions for conditional compilation
add_compile_definitions(QCPERF_PLATFORM_LINUX)

# ============================================================================
# Linux Compiler Configuration
# ============================================================================
# Configure compiler flags specific to GCC and Clang on Linux platforms
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # GCC-specific flags
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -fPIC
    )

    # Enable additional warnings
    add_compile_options(
        -Wcast-align
        -Wunused
        -Wconversion
        -Wsign-conversion
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Clang-specific flags
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -fPIC
    )
endif()

# ============================================================================
# Linux ARM64 Configuration
# ============================================================================
# Special handling for Linux on ARM64 architecture (e.g., Raspberry Pi 4, AWS Graviton)
if(QCPERF_PLATFORM_LINUX_ARM64)
    message(STATUS "Configuring for Linux ARM64")

    # Add ARM64-specific compile definitions
    add_compile_definitions(LINUX_ARM64)

    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        # Enable ARM64 specific optimizations
        add_compile_options(-march=armv8-a)
    endif()
endif()

# ============================================================================
# Linux x86_64 Configuration
# ============================================================================
# Configuration for standard 64-bit Intel/AMD platforms on Linux
if(QCPERF_PLATFORM_LINUX_X86_64)
    message(STATUS "Configuring for Linux x86_64")

    # Add x86_64-specific compile definitions if needed
    # add_compile_definitions(LINUX_X64)

    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        # Enable x86_64 specific optimizations if needed
        # add_compile_options(-march=x86-64)
    endif()
endif()

# ============================================================================
# Linux Library Dependencies
# ============================================================================
# Define Linux-specific libraries that need to be linked with the project
set(QCPERF_LINUX_LIBS "")

# Add Linux-specific libraries if needed
if(QCPERF_OS_LINUX AND NOT QCPERF_OS_ANDROID)
    list(APPEND QCPERF_LINUX_LIBS
        pthread
        dl
    )
endif()

set(_OS_M "m")
if(QCPERF_OS_ANDROID)
    set(_OS_THREAD "")
else()
    set(_OS_THREAD "pthread")
endif()

message(STATUS "Linux platform configuration loaded")
