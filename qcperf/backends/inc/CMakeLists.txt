# ============================================================================
# QcPerf Backend Interface Library
# ============================================================================
# This CMakeLists.txt defines the unified backend interface library that
# aggregates all backend implementations and provides a single point of
# dependency for the core library. It centralizes backend-specific includes
# and dependencies, allowing the core to remain decoupled from specific backends.
#
# Backend inclusion is controlled by QCPERF_ENABLED_* variables set in
# cmake/BuildConfig.cmake based on the BACKENDS CMake option and the
# detected platform.

# ============================================================================
# Interface Library Target
# ============================================================================
# Create a unified backend interface library
# This is an INTERFACE library (header-only) that serves as a facade
# for all backend implementations
add_library(QcPerfBackends INTERFACE)

# ============================================================================
# Include Directories
# ============================================================================
# Set include directories for the unified backend interface
# These are the common headers that all backends need to implement
target_include_directories(QcPerfBackends INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ============================================================================
# Per-Backend Includes and Link Libraries
# ============================================================================
# Each backend is conditionally included based on its QCPERF_ENABLED_* flag.
# To add a new backend, add a corresponding block below following the same pattern.

# Dummy backend
if(QCPERF_ENABLED_DUMMY)
    target_include_directories(QcPerfBackends INTERFACE
        ${CMAKE_SOURCE_DIR}/backends/dummy/inc
    )
    target_link_libraries(QcPerfBackends INTERFACE
        QcPerfDummyBackend
    )
endif()

# Linux ARM64 backends
if(QCPERF_ENABLED_QCOM_LINUX_CPU)
    target_include_directories(QcPerfBackends INTERFACE
        ${CMAKE_SOURCE_DIR}/backends/qcom-linux-cpu/inc
    )
    target_link_libraries(QcPerfBackends INTERFACE
        QcPerfQcomLinuxCpuBackend
    )
endif()

if(QCPERF_ENABLED_QCOM_LINUX_NPU OR QCPERF_ENABLED_WOS_NPU)
    target_include_directories(QcPerfBackends INTERFACE
        ${CMAKE_SOURCE_DIR}/backends/qcom-dsp-npu/inc
    )
    target_link_libraries(QcPerfBackends INTERFACE
        QcPerfDspNpuBackend
        QcPerfQcomDsp
    )
endif()

# Windows ARM64 backends
if(QCPERF_ENABLED_WOS_THERMAL)
    target_include_directories(QcPerfBackends INTERFACE
        ${CMAKE_SOURCE_DIR}/backends/wos-thermal/inc
    )
    target_link_libraries(QcPerfBackends INTERFACE
        QcPerfWosThermalBackend
    )
endif()

if(QCPERF_ENABLED_WOS_POWER)
    target_include_directories(QcPerfBackends INTERFACE
        ${CMAKE_SOURCE_DIR}/backends/wos-power-backend/inc
    )
    target_link_libraries(QcPerfBackends INTERFACE
        QcPerfWosPowerBackend
    )
endif()

if(QCPERF_ENABLED_WOS_CPU)
    target_include_directories(QcPerfBackends INTERFACE
        ${CMAKE_SOURCE_DIR}/backends/wos-cpu/inc
    )
    target_link_libraries(QcPerfBackends INTERFACE
        QcPerfWosCpuBackend
    )
endif()
