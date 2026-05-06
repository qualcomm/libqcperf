# qlist library CMake configuration
# Builds doubly linked list implementation

cmake_minimum_required(VERSION 3.13.5)

add_library(QcPerfqlist STATIC
    src/qlist.c
)

# Link with qmutex library for thread safety
target_link_libraries(QcPerfqlist  
    QcPerfQMutex
)

# Set position-independent code flag for non-Windows platforms
if(NOT QCPERF_OS_WINDOWS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic")
endif()

# Include public headers and qmutex headers
target_include_directories(QcPerfqlist PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/utils/qmutex/inc
)
