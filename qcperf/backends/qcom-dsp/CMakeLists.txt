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
