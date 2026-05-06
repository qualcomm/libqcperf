# qsleep library CMake configuration
# Builds platform-independent sleep implementation

cmake_minimum_required(VERSION 3.13.5)

add_library(QcPerfQSleep STATIC
    src/qsleep.c
)

# Include public headers
target_include_directories(QcPerfQSleep PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
