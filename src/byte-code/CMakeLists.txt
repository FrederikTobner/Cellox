cellox_add_library(clx_bytecode
    SOURCES
        src/chunk.c
        src/chunk_disassembler.c
        src/chunk_file.c
    DEPENDENCIES
        clx_model
    # cellox_config.h is generated into the binary dir; expose it to all consumers
    INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_BINARY_DIR}/src"
)
