# ============================================================================
# QcPerf Linux Thermal Backend
# ============================================================================
# This CMakeLists.txt builds the linux-thermal backend, which monitors thermal
# zone temperatures and cooling-device states on Linux platforms via the kernel
# thermal framework exposed under /sys/class/thermal.

message("Added linux-thermal backend")

add_library(QcPerfLinuxThermalBackend STATIC
    src/linux_thermal.c
    src/linux_thermal_info.c
    src/linux_thermal_lib.c
    src/linux_thermal_logger.c
)

target_include_directories(QcPerfLinuxThermalBackend PUBLIC
    inc
    ${CMAKE_SOURCE_DIR}/core/inc
    ${CMAKE_SOURCE_DIR}/core/inc/internal
    ${CMAKE_SOURCE_DIR}/backends/inc
    ${CMAKE_SOURCE_DIR}/utils/qthread/inc
    ${CMAKE_SOURCE_DIR}/utils/qtime/inc
)

target_link_libraries(QcPerfLinuxThermalBackend
    QcPerfQThread
    QcPerfQTime
    ${_OS_M}
)
