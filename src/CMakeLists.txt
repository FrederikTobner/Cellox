cmake_minimum_required(VERSION 3.16)

set(HEADERS
    chunk.h
    common.h
    compiler.h
    dynamicArray.h
    debug.h
    init.h
    lexer.h
    memory.h
    nativeFun.h
    object.h
    preProcessor.h
    stringUtils.h
    table.h
    value.h
    vm.h
)

set(SOURCES    
    chunk.c
    compiler.c
    compiler.h
    dynamicArray.c
    debug.c
    init.c 
    lexer.c
    main.c
    memory.c 
    nativeFun.c
    object.c 
    preProcessor.c
    stringUtils.c
    table.c
    value.c  
    vm.c
)

add_executable(Cellox ${SOURCES} ${HEADERS})