cmake_minimum_required(VERSION 3.0.0)
project(Cellox VERSION 0.1.0)

set(SOURCES
src/main.c
src/chunk.c
src/chunk.h
src/common.h
src/compiler.c
src/compiler.h
src/debug.c 
src/debug.h
src/lexer.c 
src/lexer.h
src/memory.c 
src/memory.h 
src/object.c 
src/object.h
src/table.c
src/table.h 
src/value.c 
src/value.h 
src/vm.c
src/vm.h
)

add_executable(Cellox ${SOURCES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_C_STANDARD 11)
include(CPack)
