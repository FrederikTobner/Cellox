cmake_minimum_required(VERSION 3.0.0)
project(Cellox VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(Cellox main.c chunk.c chunk.h common.h compiler.c compiler.h debug.c debug.h memory.c memory.h object.c object.h scanner.c scanner.h table.c table.h value.c value.h vm.c vm.h)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_C_STANDARD 11)
include(CPack)
