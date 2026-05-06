# qmutex library CMake configuration
# Builds platform-specific mutex implementation based on target OS

cmake_minimum_required(VERSION 3.13.5)

if(QCPERF_OS_WINDOWS)
    add_library(QcPerfQMutex STATIC
        src/windows/qmutex.c
    )
else()
    add_library(QcPerfQMutex STATIC
        src/linux/qmutex.c
    )
endif()

# Include public headers
target_include_directories(QcPerfQMutex PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
