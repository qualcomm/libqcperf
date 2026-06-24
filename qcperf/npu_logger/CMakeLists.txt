# Build QcPerfNpuLogger only when explicitly requested via -DBUILD_NPU_LOGGER=ON
# and the NPU backend is compiled in.  Off by default because the daemon
# requires libcdsprpc.so at runtime and is only meaningful on Qualcomm hardware.
option(BUILD_NPU_LOGGER "Build the QcPerfNpuLogger systemd daemon" OFF)

if(BUILD_NPU_LOGGER AND QCPERF_ENABLED_QCOM_LINUX_NPU)

    add_executable(QcPerfNpuLogger
        src/qcperf_npu_logger.c
    )

    set_target_properties(QcPerfNpuLogger PROPERTIES
        OUTPUT_NAME "QcPerfNpuLogger"
        POSITION_INDEPENDENT_CODE OFF
    )

    target_include_directories(QcPerfNpuLogger
        PUBLIC
            ${CMAKE_SOURCE_DIR}/core/inc
    )

    target_link_libraries(QcPerfNpuLogger
        PUBLIC
            QcPerfCore
    )

    # When linking statically on Linux ARM64 with the NPU backend enabled,
    # the cdsprpc shared library must be linked explicitly (same pattern as
    # test_app/CMakeLists.txt).
    if(NOT BUILD_SHARED AND QCPERF_LINUX_ARM64 AND QCPERF_ENABLED_QCOM_LINUX_NPU)
        target_link_libraries(QcPerfNpuLogger PRIVATE
            cdsprpc
        )
    endif()

    target_compile_options(QcPerfNpuLogger PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    # Install the daemon binary alongside QcPerfCoreTest.
    install(TARGETS QcPerfNpuLogger
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    # Install the systemd unit file into the standard system unit directory.
    install(FILES
        systemd/qcperf-npu-logger.service
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/system
    )

else()
    if(BUILD_NPU_LOGGER AND NOT QCPERF_ENABLED_QCOM_LINUX_NPU)
        message(WARNING "BUILD_NPU_LOGGER=ON but NPU backend is not enabled — QcPerfNpuLogger will not be built. Add NPU to -DBACKENDS.")
    else()
        message(STATUS "QcPerfNpuLogger: disabled (pass -DBUILD_NPU_LOGGER=ON to enable)")
    endif()

endif()
