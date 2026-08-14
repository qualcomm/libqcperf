# ============================================================================
# QcPerf Backends
# ============================================================================
# This CMakeLists.txt manages all backend implementations for the QcPerf library.
# It includes individual backend subdirectories and sets up the unified backend
# interface that the core library depends on.
#
# The QcPerf library uses a modular backend architecture where each backend
# provides specific performance monitoring capabilities:
#
# - dummy: A reference implementation that generates synthetic metrics for testing
# - wos-thermal: Monitors temperature and passive cooling for various thermal zones
# - wos-power-backend: Monitors power consumption metrics
#
# Each backend implements a common interface defined in the inc/ directory,
# allowing the core library to interact with any backend in a uniform way.
# Backend inclusion is controlled by QCPERF_ENABLED_* variables set in
# cmake/BuildConfig.cmake based on the BACKENDS CMake option and the
# detected platform.

# ============================================================================
# Backend Subdirectories
# ============================================================================
# Add subdirectories for individual backends, guarded by QCPERF_ENABLED_* flags.
# To add a new backend:
#   1. Add its name to QCPERF_PLATFORM_SUPPORTED_BACKENDS in cmake/BuildConfig.cmake
#   2. Add the corresponding add_subdirectory call below

if(QCPERF_ENABLED_DUMMY)
    add_subdirectory(dummy)
endif()

# Add utility libraries used by backends
add_subdirectory(utils)

# Linux ARM64 backends
if(QCPERF_ENABLED_QCOM_LINUX_CPU)
    add_subdirectory(qcom-linux-cpu)
endif()

if(QCPERF_ENABLED_QCOM_LINUX_NPU)
    add_subdirectory(qcom-dsp)
    add_subdirectory(qcom-dsp-npu)
endif()

if(QCPERF_ENABLED_QCOM_LINUX_THERMAL)
    add_subdirectory(linux-thermal)
endif()

# Windows ARM64 backends
if(QCPERF_ENABLED_WOS_THERMAL)
    add_subdirectory(wos-thermal)
endif()

if(QCPERF_ENABLED_WOS_POWER)
    add_subdirectory(wos-power-backend)
endif()

if(QCPERF_ENABLED_WOS_CPU)
    add_subdirectory(wos-cpu)
endif()

# ============================================================================
# Backend Interface
# ============================================================================
# Add the inc directory which contains the unified backend interface
# This creates an interface library that aggregates all backends
add_subdirectory(inc)
