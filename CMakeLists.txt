cmake_minimum_required(VERSION 3.16)

project(Cellox 
VERSION 0.1.0 
LANGUAGES C CXX
)


# C99 standard is required to build the compiler
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# C++ 14 standard is required for the google test framework
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)  

# Optional NAN-Boxing (Not a number boxing / Not a number tagging)
option(CLX_NAN_BOXING_ACTIVATED "Determines whether \"not a number boxing / tagging\" is used" ON)

# Debug options (only have an effect on debug builds)
option(CLX_DEBUG_PRINT_BYTECODE "Determines whether the chunks are dissassembled and the bytecode is printed" OFF)
option(CLX_DEBUG_TRACE_EXECUTION "Determines whether the execution shall be traced" OFF)
option(CLX_DEBUG_STRESS_GARBAGE_COLLECTOR "Determines whether the garbage collector shall be stressed" OFF)
option(CLX_DEBUG_LOG_GARBAGE_COLLECTION "Determines whether the garbage collection be logged" OFF)

# Build options
option(CLX_BUILD_TESTS "Determines whether the tests shall be built" OFF)
option(CLX_BUILD_TOOLS "Determines whether the development tools shall be built" ON)

include(CheckIncludeFile)

get_filename_component(SOURCEPATH ${PROJECT_SOURCE_DIR}/src ABSOLUTE)

# Not a number boxing affects all parts of the project
if(CLX_NAN_BOXING_ACTIVATED)
    add_compile_definitions(NAN_BOXING)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(CompileDefinitions)

cellox_add_compiler_compile_definitions()
cellox_add_os_compile_definitions()

file(GLOB_RECURSE CELLOX_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE CELLOX_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

LIST(REMOVE_ITEM CELLOX_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/common.c )
LIST(REMOVE_ITEM CELLOX_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/common.h)

# Builds the cellox interpreter
add_subdirectory(src)

LIST(REMOVE_ITEM CELLOX_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c CELLOX_SOURCE_FILES )

if(CLX_BUILD_TOOLS)
    # Builds the benchmark runner tool for the cellox compiler
    add_subdirectory(benchmark)
    # Builds the disassmbler tool (for cellox chunk files)
    add_subdirectory(disassembler)
endif()

if(CLX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
