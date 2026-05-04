set(_runtime_deps clx_bytecode)
if(UNIX)
    list(APPEND _runtime_deps m)
endif()

cellox_add_library(clx_runtime
    SOURCES
        src/garbage_collector.c
        src/memory_mutator.c
        src/native_functions.c
        src/virtual_machine.c
    DEPENDENCIES
        clx_utils
        ${_runtime_deps}
    INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
