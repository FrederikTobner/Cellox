cmake_minimum_required(VERSION 3.0.0)
project(Cellox VERSION 0.1.0)

add_executable(Cellox 
src/main.c 
src/chunk.c
src/chunk.h
src/common.h
src/compiler.c
src/compiler.h
src/debug.c 
src/debug.h 
src/memory.c 
src/memory.h 
src/object.c 
src/object.h 
src/scanner.c 
src/scanner.h 
src/table.c 
src/table.h 
src/value.c 
src/value.h 
src/vm.c 
src/vm.h)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_C_STANDARD 11)
include(CPack)
