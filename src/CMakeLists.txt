cmake_minimum_required(VERSION 3.16)

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

add_executable(Cellox ${SOURCES} ${HEADERS})