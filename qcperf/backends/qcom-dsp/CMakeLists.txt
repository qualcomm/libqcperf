add_library(QcPerfQcomDsp STATIC
    src/qcom_dsp.c
    src/dspquery_stub.c

)

set(FASTRPC_DIR "${CMAKE_SOURCE_DIR}/third-party/fastrpc")

find_path(FASTRPC_INCLUDE_DIR AEEStdDef.h
    HINTS "${FASTRPC_DIR}/inc"
    NO_CMAKE_FIND_ROOT_PATH
)

find_library(FASTRPC_LIBRARY NAMES cdsprpc
    HINTS "${FASTRPC_DIR}/src/.libs"
    NO_CMAKE_FIND_ROOT_PATH
)

target_include_directories(QcPerfQcomDsp PUBLIC
    inc
    ${FASTRPC_INCLUDE_DIR}
)

target_link_libraries(QcPerfQcomDsp PUBLIC
    ${FASTRPC_LIBRARY}
)

if(QCPERF_OS_ANDROID)
    # libcdsprpc.so (built from the fastrpc submodule) references
    # __android_log_print but is not itself linked against liblog, so
    # anything that depends on libcdsprpc.so (static or shared QcPerfCore
    # builds alike) must supply liblog to satisfy Android's
    # --no-allow-shlib-undefined linker default.
    target_link_libraries(QcPerfQcomDsp PUBLIC log)
endif()
