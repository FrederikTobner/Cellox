cellox_add_library(clx_utils
    SOURCES
        src/string_utils.c
    DEPENDENCIES
        clx_base
    INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
