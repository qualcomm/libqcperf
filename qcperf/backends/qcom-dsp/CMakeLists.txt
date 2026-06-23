add_library(QcPerfQcomDsp SHARED
    src/qcom_dsp.c
    src/dspquery_stub.c
    
)

find_path(FASTRPC_INCLUDE_DIR AEEStdDef.h
    PATHS /usr/include/fastrpc /usr/include /usr/local/include/fastrpc /usr/include/aarch64-linux-gnu
)

find_library(FASTRPC_LIBRARY NAMES cdsprpc
    PATHS /usr/lib /usr/local/lib /usr/lib/aarch64-linux-gnu/
)

target_include_directories(QcPerfQcomDsp PUBLIC
    inc
    ${FASTRPC_INCLUDE_DIR}
)

target_link_libraries(QcPerfQcomDsp PUBLIC 
    ${FASTRPC_LIBRARY}
)
