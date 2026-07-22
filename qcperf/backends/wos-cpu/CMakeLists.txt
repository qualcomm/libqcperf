# ============================================================================
# QcPerf WOS CPU Backend
# ============================================================================
# This CMakeLists.txt builds the Windows on Snapdragon (WoS) CPU backend for
# the QcPerf library. This backend monitors CPU performance metrics including
# per-core utilization, frequency, and effective utilization using Windows PDH
# (Performance Data Helper) APIs.
#
# This backend is only built for Windows ARM64 platforms.

message("Added wos-cpu backend")

add_library(QcPerfWosCpuBackend STATIC
    src/wos_cpu.c
    src/wos_cpu_logger.c
)

add_subdirectory(cpu-telemetry)

target_include_directories(QcPerfWosCpuBackend PUBLIC
    inc
    cpu-telemetry/inc
    ${CMAKE_SOURCE_DIR}/backends/inc/
    ${CMAKE_SOURCE_DIR}/core/inc
    ${CMAKE_SOURCE_DIR}/core/inc/internal/
    ${CMAKE_SOURCE_DIR}/utils/qthread/inc
    ${CMAKE_SOURCE_DIR}/utils/qtime/inc
)

target_link_libraries(QcPerfWosCpuBackend
    QcPerfQThread
    QcPerfQTime
    QcWosCpuTelemetry
)
