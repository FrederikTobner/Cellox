cellox_add_library(clx_base
    SOURCES
        src/common.c
    DEPENDENCIES
        clx_toolchain
    INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
