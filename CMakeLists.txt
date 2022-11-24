cmake_minimum_required(VERSION 3.16)
project(Cellox
VERSION 0.1.0
)
include(CheckIncludeFile)

# C99 standard is required to build the interpreter
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# raltive path to the sources of the cellox interpreter (for the tests and the benchmarking suite)
set(SOURCEPATH ../src/)

# source files of the interpreter
set(CELLOXSOURCES
${SOURCEPATH}chunk_file.c
${SOURCEPATH}chunk.c
${SOURCEPATH}command_line_argument_parser.c
${SOURCEPATH}compiler.c
${SOURCEPATH}compiler.c
${SOURCEPATH}dynamic_value_array.c
${SOURCEPATH}debug.c
${SOURCEPATH}hash_table.c
${SOURCEPATH}init.c
${SOURCEPATH}lexer.c
${SOURCEPATH}memory.c
${SOURCEPATH}native_functions.c
${SOURCEPATH}object.c
${SOURCEPATH}string_utils.c
${SOURCEPATH}value.c
${SOURCEPATH}virtual_machine.c
)

# headers of the interpreter
set(CELLOXHEADERS
${SOURCEPATH}chunk_file.h
${SOURCEPATH}chunk.h
${SOURCEPATH}command_line_argument_parser.h
${SOURCEPATH}compiler.h
${SOURCEPATH}dynamic_value_array.h
${SOURCEPATH}debug.h
${SOURCEPATH}hash_table.h
${SOURCEPATH}init.h
${SOURCEPATH}lexer.h
${SOURCEPATH}memory.h
${SOURCEPATH}native_functions.h
${SOURCEPATH}object.h
${SOURCEPATH}string_utils.h
${SOURCEPATH}value.h
${SOURCEPATH}virtual_machine.h
)

if(UNIX)
CHECK_INCLUDE_FILE("curses.h" CURSES_AVAILABLE)
if(NOT ${CURSES_AVAILABLE})
message(FATAL_ERROR "curses.h is required to build the interpreter under unix-like systems. Please install it and try again after that")
endif() # Curses not available
endif() # Unix system

# Builds the cellox interpreter
add_subdirectory(src)

# Builds benchmarks for the cellox interpreter
add_subdirectory(benchmark)

# Please use msvc for testing (building googletest fails using gcc) and specify one as the number of parallel test jobs ("-j 1") when you execute the tests
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
else()
enable_testing()
# Builds tests for the interpreter
add_subdirectory(test EXCLUDE_FROM_ALL)
endif()