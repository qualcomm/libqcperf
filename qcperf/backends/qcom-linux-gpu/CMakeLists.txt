# ============================================================================
# QcPerf Qualcomm Linux GPU Backend
# ============================================================================
# Reports Adreno GPU utilization and clock from the kgsl sysfs nodes.

message("Added qcom-linux-gpu backend")

add_library(QcPerfQcomLinuxGpuBackend STATIC
    src/qcom_linux_gpu.c
    src/qcom_linux_gpu_info.c
    src/qcom_linux_gpu_utils.c
)

target_include_directories(QcPerfQcomLinuxGpuBackend PUBLIC
    inc
    ${CMAKE_SOURCE_DIR}/core/inc
    ${CMAKE_SOURCE_DIR}/core/inc/internal
    ${CMAKE_SOURCE_DIR}/backends/inc
    ${CMAKE_SOURCE_DIR}/utils/qthread/inc
    ${CMAKE_SOURCE_DIR}/utils/qtime/inc
)

target_link_libraries(QcPerfQcomLinuxGpuBackend
    QcPerfQThread
    QcPerfQTime
    ${_OS_M}
)
