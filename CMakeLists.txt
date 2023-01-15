cmake_minimum_required(VERSION 3.16)
project(Cellox VERSION 0.1.0)
include(CheckIncludeFile)

# C99 standard is required to build the interpreter
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# CPack metadata
set(PROJECT_PAGE "https://github.com/FrederikTobner/Cellox")
set(PROJECT_HELP_PAGE "https://github.com/FrederikTobner/Cellox/wiki")
set(PROJECT_VENDOR "Frederik Tobner")

# Optional NAN-Boxing (Not a number boxing / Not a number tagging)
option(NAN_BOXING_ACTIVATED "Determines whether \"not a number boxing / tagging\" is used" ON)

# Debug options (only have an effect on debug builds)
option(DEBUG_PRINT_BYTECODE "Determines whether the chunks are dissassembled and the bytecode is printed" OFF)
option(DEBUG_TRACE_EXECUTION "Determines whether the execution shall be traced" OFF)
option(DEBUG_STRESS_GARBAGE_COLLECTOR "Determines whether the garbage collector shall be stressed" OFF)
option(DEBUG_LOG_GARBAGE_COLLECTION "Determines whether the garbage collection be logged" OFF)

# Getting absolute file path of the project root and src directory
get_filename_component(PROJECT_DIRECTORY ./ ABSOLUTE)
get_filename_component(SOURCEPATH ./src ABSOLUTE)     

# Check dependecies under unix-like systems
if(UNIX)
    CHECK_INCLUDE_FILE("curses.h" CURSES_AVAILABLE)
    CHECK_INCLUDE_FILE("unistd.h" UNISTD_AVAILABLE)
    if(NOT ${CURSES_AVAILABLE})
        message(FATAL_ERROR "curses.h is required to build the interpreter under unix-like systems. Please install it and try again after that")
    endif() # curses.h not available
    if(NOT ${UNISTD_AVAILABLE})
        message(FATAL_ERROR "unistd.h is required to build the interpreter under unix-like systems. Please install it and try again after that")
    endif() # unistd.h not available
endif() # Unix system

# Check dependecies under windows
if(WIN32)
    CHECK_INCLUDE_FILE("conio.h" CONIO_AVAILABLE)
    CHECK_INCLUDE_FILE("windows.h" WINDOWS_AVAILABLE)
    if(NOT ${CONIO_AVAILABLE})
        message(FATAL_ERROR "conio.h is required to build the interpreter under windows. Please install it and try again after that")
    endif() # conio.h not available
    if(NOT ${WINDOWS_AVAILABLE})
        message(FATAL_ERROR "windows.h is required to build the interpreter under windows. Please install it and try again after that")
    endif() # windows.h not available
endif() # Windows system

# source files of the interpreter
set(CELLOX_SOURCE_FILES
"${SOURCEPATH}/chunk_file.c"
"${SOURCEPATH}/chunk.c"
"${SOURCEPATH}/command_line_argument_parser.c"
"${SOURCEPATH}/compiler.c"
"${SOURCEPATH}/compiler.c"
"${SOURCEPATH}/dynamic_value_array.c"
"${SOURCEPATH}/debug.c"
"${SOURCEPATH}/garbage_collector.c"
"${SOURCEPATH}/hash_table.c"
"${SOURCEPATH}/initializer.c"
"${SOURCEPATH}/lexer.c"
"${SOURCEPATH}/memory_mutator.c"
"${SOURCEPATH}/native_functions.c"
"${SOURCEPATH}/object.c"
"${SOURCEPATH}/string_utils.c"
"${SOURCEPATH}/value.c"
"${SOURCEPATH}/virtual_machine.c"
)

# Headers of the interpreter
set(CELLOX_HEADER_FILES
"${SOURCEPATH}/chunk_file.h"
"${SOURCEPATH}/chunk.h"
"${SOURCEPATH}/command_line_argument_parser.h"
"${SOURCEPATH}/compiler.h"
"${SOURCEPATH}/dynamic_value_array.h"
"${SOURCEPATH}/debug.h"
"${SOURCEPATH}/garbage_collector.h"
"${SOURCEPATH}/hash_table.h"
"${SOURCEPATH}/initializer.h"
"${SOURCEPATH}/lexer.h"
"${SOURCEPATH}/memory_mutator.h"
"${SOURCEPATH}/native_functions.h"
"${SOURCEPATH}/object.h"
"${SOURCEPATH}/string_utils.h"
"${SOURCEPATH}/value.h"
"${SOURCEPATH}/virtual_machine.h"
)

if(NAN_BOXING_ACTIVATED)
    add_compile_definitions(NAN_BOXING)
endif()

# Adding os-specific compiler definitions
if(UNIX)
    add_compile_definitions(OS_UNIX_LIKE)    
    if(LINUX) 
        add_compile_definitions(OS_LINUX)
    elseif(APPLE) 
        add_compile_definitions(OS_MACOS)
    else()
        add_compile_definitions(OS_UNKNOWN_UNIX)
    endif()
elseif(WIN32)
    add_compile_definitions(OS_WINDOWS)
else()
    add_compile_definitions(OS_UNKNOWN)
endif()
  

# We determine the compiler so we can do some optimization for a specific compiler
if(CMAKE_C_COMPILER_ID STREQUAL "[Gg][Nn][Uu]")
    add_compile_definitions(COMPILER_GCC)
elseif(CMAKE_C_COMPILER_ID STREQUAL "[Cc][Ll][Aa][Nn][Gg]")
    add_compile_definitions(COMPILER_GCC)
elseif(CMAKE_C_COMPILER_ID STREQUAL "[Mm][Ss][Vv][Cc]")
    add_compile_definitions(COMPILER_MSVC)
else()
    add_compile_definitions(COMPILER_UNKNOWN)    
endif()

# Builds the cellox interpreter
add_subdirectory(src)

# Builds benchmarks for the cellox interpreter
add_subdirectory(benchmark)

# Builds the disassmbler (for cellox chunk files)
add_subdirectory(disassembler)

# Please use msvc for testing (building googletest fails using gcc) and specify one as the number of parallel test jobs ("-j 1") when you execute the tests.
if(MSVC)
    enable_testing()
    # Builds tests for the interpreter (add exclude from all if the project is packaged)
    add_subdirectory(test)
endif()
