# configure cellox config to show version
configure_file(cellox_config.h.in cellox_config.h)

set(HEADERS
    chunk.h
    common.h
    compiler.h
    dynamic_array.h
    debug.h
    init.h
    lexer.h
    memory.h
    native_fun.h
    object.h
    string_utils.h
    table.h
    value.h
    virtual_machine.h
)

set(SOURCES
    chunk.c
    compiler.c
    compiler.h
    dynamic_array.c
    debug.c
    init.c 
    lexer.c
    main.c
    memory.c 
    native_fun.c
    object.c 
    string_utils.c
    table.c
    value.c  
    virtual_machine.c
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# for including the cellox_config.h file
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR}/src)
