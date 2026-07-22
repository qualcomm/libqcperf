# ============================================================================
# QcPerf WOS CPU Telemetry Library
# ============================================================================
# Static library providing CPU performance data collection via
# NtQuerySystemInformation + CallNtPowerInformation.

add_library(QcWosCpuTelemetry STATIC
    src/cpu_telemetry_ntquery.c
    src/cpu_telemetry_pdh.c
)

target_include_directories(QcWosCpuTelemetry PUBLIC
    inc
)

target_link_libraries(QcWosCpuTelemetry
    pdh
    PowrProf
)
