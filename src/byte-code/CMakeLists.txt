cellox_add_library(clx_bytecode
    SOURCES
        chunk.c
        chunk_disassembler.c
        chunk_file.c
    DEPENDENCIES
        clx_model
    # cellox_config.h is generated into the binary dir; expose it to all consumers
    INCLUDE_DIRS
        "${CMAKE_BINARY_DIR}/src"
)
