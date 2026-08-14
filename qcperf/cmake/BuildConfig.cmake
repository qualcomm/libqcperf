# ============================================================================
# QcPerf - Combined Build Configuration
# ============================================================================
# This file centralizes all build configuration for the QcPerf library:
# - Platform detection (OS and architecture)
# - Platform-specific compiler flags and definitions
# - Compiler configuration and optimization settings
# - Platform types and standard header checks
#
# It is designed to support multiple platforms (Windows, Linux) and
# architectures (x86_64, ARM64) with a focus on Windows ARM64 support.
# ============================================================================

# ============================================================================
# Platform Detection
# ============================================================================
# Detect operating system and architecture to set appropriate build flags
# and definitions. This enables conditional compilation for platform-specific
# code throughout the project.

# Detect OS
if(WIN32)
    set(QCPERF_OS_WINDOWS TRUE)
    message(STATUS "Detected Windows platform")
elseif(ANDROID)
    set(QCPERF_OS_LINUX TRUE)
    set(QCPERF_OS_ANDROID TRUE)
    message(STATUS "Detected Android platform")
elseif(UNIX)
    set(QCPERF_OS_LINUX TRUE)
    message(STATUS "Detected Linux platform")
else()
    message(WARNING "Unknown platform detected")
endif()

# Detect architecture
# For Visual Studio, we need to check CMAKE_VS_PLATFORM_NAME
if(MSVC AND CMAKE_VS_PLATFORM_NAME MATCHES "ARM64")
    set(QCPERF_ARCH_ARM64 TRUE)
    # Override CMAKE_SYSTEM_PROCESSOR for Visual Studio ARM64 builds
    set(CMAKE_SYSTEM_PROCESSOR "ARM64")
    message(STATUS "Detected ARM64 architecture (Visual Studio)")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64")
    set(QCPERF_ARCH_X86_64 TRUE)
    message(STATUS "Detected x86_64 architecture")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64|arm64")
    set(QCPERF_ARCH_ARM64 TRUE)
    message(STATUS "Detected ARM64 architecture")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686|i386")
    set(QCPERF_ARCH_X86 TRUE)
    message(STATUS "Detected x86 architecture")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7|armhf")
    set(QCPERF_ARCH_ARM32 TRUE)
    message(STATUS "Detected ARM32 architecture")
else()
    message(STATUS "Architecture detection using CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
    if(DEFINED CMAKE_VS_PLATFORM_NAME)
        message(STATUS "Visual Studio platform name: ${CMAKE_VS_PLATFORM_NAME}")
    endif()
    message(WARNING "Unknown architecture - defaulting to x86_64")
    set(QCPERF_ARCH_X86_64 TRUE)
endif()

# Set combined platform variables for convenience
if(QCPERF_OS_WINDOWS AND QCPERF_ARCH_ARM64)
    set(QCPERF_PLATFORM_WINDOWS_ARM64 TRUE)
elseif(QCPERF_OS_WINDOWS AND QCPERF_ARCH_X86_64)
    set(QCPERF_PLATFORM_WINDOWS_X86_64 TRUE)
elseif(QCPERF_OS_LINUX AND QCPERF_ARCH_ARM64)
    set(QCPERF_PLATFORM_LINUX_ARM64 TRUE)
elseif(QCPERF_OS_LINUX AND QCPERF_ARCH_X86_64)
    set(QCPERF_PLATFORM_LINUX_X86_64 TRUE)
endif()

# Print platform summary
message(STATUS "QcPerf Platform Configuration:")
message(STATUS "  OS: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
if(CMAKE_CROSSCOMPILING)
    message(STATUS "  Cross-compiling: YES (host=${CMAKE_HOST_SYSTEM_PROCESSOR} -> target=${CMAKE_SYSTEM_PROCESSOR})")
    message(STATUS "  C compiler: ${CMAKE_C_COMPILER}")
else()
    message(STATUS "  Cross-compiling: NO (native build)")
endif()

# ============================================================================
# Platform-Specific Configurations
# ============================================================================
# Apply platform-specific compiler flags, definitions, and optimizations
# based on the detected OS and architecture combinations.

# Windows-specific configurations
if(QCPERF_OS_WINDOWS)
    # Enable Windows-specific definitions
    add_compile_definitions(QCPERF_PLATFORM_WINDOWS)

    # Windows-specific compiler flags
    if(MSVC)
        # Enable multi-processor compilation
        add_compile_options(/MP)

        # Warning level
        add_compile_options(/W4)

        # Disable specific warnings if needed
        # add_compile_options(/wd4100) # Unused parameter

        # Enable additional security features
        add_compile_definitions(
            _CRT_SECURE_NO_WARNINGS
            _SCL_SECURE_NO_WARNINGS
        )
    endif()

    # ARM64 specific configurations
    if(QCPERF_PLATFORM_WINDOWS_ARM64)
        message(STATUS "Configuring for Windows ARM64")

        # Add ARM64-specific compile definitions
        add_compile_definitions(WIN32_ARM64)
    endif()

    # x86_64 specific configurations
    if(QCPERF_PLATFORM_WINDOWS_X86_64)
        message(STATUS "Configuring for Windows x86_64")
    endif()

    message(STATUS "Windows platform configuration loaded")
endif()

# ============================================================================
# Backend Selection
# ============================================================================
# Determine which backends are supported on the current platform.
# For each OS-specific backend, QCPERF_BACKEND_OS_PREFIX_<NAME> is set to the
# OS qualifier (e.g. QCOM_LINUX, WOS) so the compile definition becomes
# QCPERF_ENABLED_<OS>_<NAME>. DUMMY has no OS prefix and remains QCPERF_ENABLED_DUMMY.
set(QCPERF_PLATFORM_SUPPORTED_BACKENDS "DUMMY")

if(QCPERF_PLATFORM_LINUX_ARM64)
    set(_OS_PREFIX "QCOM_LINUX")
    foreach(_BACKEND CPU NPU THERMAL)
        list(APPEND QCPERF_PLATFORM_SUPPORTED_BACKENDS ${_BACKEND})
        set(QCPERF_BACKEND_OS_PREFIX_${_BACKEND} ${_OS_PREFIX})
    endforeach()
endif()

if(QCPERF_PLATFORM_WINDOWS_ARM64)
    set(_OS_PREFIX "WOS")
    foreach(_BACKEND THERMAL POWER CPU)
        list(APPEND QCPERF_PLATFORM_SUPPORTED_BACKENDS ${_BACKEND})
        set(QCPERF_BACKEND_OS_PREFIX_${_BACKEND} ${_OS_PREFIX})
    endforeach()
endif()

# If empty (default), all platform-supported backends are enabled.
set(BACKENDS "" CACHE STRING
    "Semicolon-separated list of backends to enable (e.g. \"DUMMY;CPU;NPU\"). \
If empty, all platform-supported backends are enabled.")

if(BACKENDS)
    # User specified an explicit list — enable only those supported on this platform
    foreach(_BACKEND_INPUT ${BACKENDS})
        string(TOUPPER "${_BACKEND_INPUT}" _BACKEND)
        if(${_BACKEND} IN_LIST QCPERF_PLATFORM_SUPPORTED_BACKENDS)
            if(DEFINED QCPERF_BACKEND_OS_PREFIX_${_BACKEND})
                set(_OS_PREFIX ${QCPERF_BACKEND_OS_PREFIX_${_BACKEND}})
                set(QCPERF_ENABLED_${_OS_PREFIX}_${_BACKEND} ON)
                message(STATUS "Backend enabled (user selection): ${_OS_PREFIX}_${_BACKEND}")
            else()
                set(QCPERF_ENABLED_${_BACKEND} ON)
                message(STATUS "Backend enabled (user selection): ${_BACKEND}")
            endif()
        else()
            message(WARNING "Backend '${_BACKEND}' is not supported on this platform — skipping.")
        endif()
    endforeach()
else()
    # Default: enable all platform-supported backends
    foreach(_BACKEND ${QCPERF_PLATFORM_SUPPORTED_BACKENDS})
        if(DEFINED QCPERF_BACKEND_OS_PREFIX_${_BACKEND})
            set(_OS_PREFIX ${QCPERF_BACKEND_OS_PREFIX_${_BACKEND}})
            set(QCPERF_ENABLED_${_OS_PREFIX}_${_BACKEND} ON)
            message(STATUS "Backend enabled (platform default): ${_OS_PREFIX}_${_BACKEND}")
        else()
            set(QCPERF_ENABLED_${_BACKEND} ON)
            message(STATUS "Backend enabled (platform default): ${_BACKEND}")
        endif()
    endforeach()
endif()

# Propagate enabled backends as compile definitions for use in source code.
# This loop is automatic — no changes needed here when adding a new backend;
# just add the backend name to the appropriate platform section above.
foreach(_BACKEND ${QCPERF_PLATFORM_SUPPORTED_BACKENDS})
    if(DEFINED QCPERF_BACKEND_OS_PREFIX_${_BACKEND})
        set(_OS_PREFIX ${QCPERF_BACKEND_OS_PREFIX_${_BACKEND}})
        set(_COMPILE_FLAG QCPERF_ENABLED_${_OS_PREFIX}_${_BACKEND})
    else()
        set(_COMPILE_FLAG QCPERF_ENABLED_${_BACKEND})
    endif()
    if(${_COMPILE_FLAG})
        add_compile_definitions(${_COMPILE_FLAG})
    endif()
endforeach()

message(STATUS "")
message(STATUS "Enabled backends: ${QCPERF_PLATFORM_SUPPORTED_BACKENDS}")

# Linux-specific configurations
if(QCPERF_OS_LINUX)
    # Enable Linux-specific definitions
    add_compile_definitions(QCPERF_PLATFORM_LINUX)

    # Linux-specific compiler flags
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

    # ARM64 specific configurations
    if(QCPERF_PLATFORM_LINUX_ARM64)
        message(STATUS "Configuring for Linux ARM64")

        # Add ARM64-specific compile definitions
        add_compile_definitions(LINUX_ARM64)

        if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
            # Enable ARM64 specific optimizations
            add_compile_options(-march=armv8-a)
        endif()
    endif()

    if(QCPERF_PLATFORM_LINUX_X86_64)
        message(STATUS "Configuring for Linux x86_64")

        # Add x86_64-specific compile definitions if needed
        add_compile_definitions(LINUX_X86_64)
    endif()

    set(_OS_M "m")
    if(QCPERF_OS_ANDROID)
        set(_OS_THREAD "")
    else()
        set(_OS_THREAD "pthread")
    endif()

    message(STATUS "Linux platform configuration loaded")
endif()

# ============================================================================
# Compiler Configuration
# ============================================================================
# Configure compiler settings that apply across all platforms:
# - C language standard
# - Build type (Debug/Release) specific optimizations
# - Warning levels and error handling
# - Code generation options

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Common compiler flags for all platforms
if(MSVC)
    # MSVC-specific compiler flags
    add_compile_options(
        /W4 # Warning level 4
        /WX- # Don't treat warnings as errors (can be changed to /WX if needed)
        /MP # Multi-processor compilation
        /EHsc # Standard C++ exception handling
        /permissive- # Enforce standards conformance
    )

    # Debug-specific flags
    add_compile_options(
        $<$<CONFIG:Debug>:/Zi>   # Generate debug info
        $<$<CONFIG:Debug>:/Od>   # Disable optimization
        $<$<CONFIG:Debug>:/RTC1> # Runtime error checks
    )

    # Release-specific flags
    add_compile_options(
        $<$<CONFIG:Release>:/O2> # Optimize for speed
        $<$<CONFIG:Release>:/Oi> # Enable intrinsic functions
        $<$<CONFIG:Release>:/GL> # Whole program optimization
    )
    add_link_options(
        $<$<CONFIG:Release>:/LTCG> # Link-time code generation
    )

else()
    # GCC/Clang compiler flags
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
    )

    # Debug-specific flags
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(
            -g # Generate debug info
            -O0 # No optimization
        )
    endif()

    # Release-specific flags
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(
            -O3 # Optimize for speed
        )
    endif()
endif()

# Enable Position Independent Code for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message(STATUS "Compiler configuration loaded")

# ============================================================================
# Platform Types
# ============================================================================
# Check for standard headers and define fallback types if needed.
# This ensures consistent type definitions across different platforms
# and compiler implementations.

# When cross-compiling, skip host-based header probes and assume standard
# C11 headers are present in the cross-toolchain (ARM GNU Toolchain ships
# all standard headers). When building natively, probe the host compiler.
if(NOT CMAKE_CROSSCOMPILING)
    include(CheckIncludeFile)
    CHECK_INCLUDE_FILE(stdbool.h  HAVE_STDBOOL_H)
    CHECK_INCLUDE_FILE(stdint.h   HAVE_STDINT_H)
    CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
    CHECK_INCLUDE_FILE(stddef.h   HAVE_STDDEF_H)
else()
    # Cross-compiling: ARM GNU Toolchain ships all standard C11 headers
    message(STATUS "Cross-compiling: skipping host header checks, assuming standard C11 headers present")
    set(HAVE_STDBOOL_H  TRUE)
    set(HAVE_STDINT_H   TRUE)
    set(HAVE_INTTYPES_H TRUE)
    set(HAVE_STDDEF_H   TRUE)
endif()

if(HAVE_STDBOOL_H)
    add_compile_definitions(HAVE_STDBOOL_H)
else()
    # Define bool type if stdbool.h is not available
    add_compile_definitions(
        bool=int
        true=1
        false=0
    )
endif()

# Add appropriate definitions based on available headers
if(HAVE_STDINT_H)
    add_compile_definitions(HAVE_STDINT_H)
endif()

if(HAVE_INTTYPES_H)
    add_compile_definitions(HAVE_INTTYPES_H)
endif()

if(HAVE_STDDEF_H)
    add_compile_definitions(HAVE_STDDEF_H)
endif()

message(STATUS "Platform types configuration loaded")
