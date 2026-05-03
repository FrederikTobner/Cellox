set(_runtime_deps clx_bytecode)
if(UNIX)
    list(APPEND _runtime_deps m)
endif()

cellox_add_library(clx_runtime
    SOURCES
        garbage_collector.c
        memory_mutator.c
        native_functions.c
        virtual_machine.c
    DEPENDENCIES
        ${_runtime_deps}
)
