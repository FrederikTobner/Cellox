cmake_minimum_required(VERSION 3.16)

project(Cellox 
VERSION 0.1.0 
LANGUAGES C CXX
)

# Optional NAN-Boxing (Not a number boxing / Not a number tagging)
option(CLX_NAN_BOXING_ACTIVATED "Determines whether \"not a number boxing / tagging\" is used" ON)

# Debug options (only have an effect on debug builds)
option(CLX_DEBUG_PRINT_BYTECODE "Determines whether the chunks are dissassembled and the bytecode is printed" OFF)
option(CLX_DEBUG_TRACE_EXECUTION "Determines whether the execution shall be traced" OFF)
option(CLX_DEBUG_STRESS_GARBAGE_COLLECTOR "Determines whether the garbage collector shall be stressed" OFF)
option(CLX_DEBUG_LOG_GARBAGE_COLLECTION "Determines whether the garbage collection be logged" OFF)

# Build options
option(CLX_BUILD_TESTS "Determines whether the tests shall be built" ON)
option(CLX_BUILD_TOOLS "Determines whether the development tools shall be built" ON)

include(CheckIncludeFile)

# C99 standard is required to build the compiler
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

get_filename_component(SOURCEPATH ${PROJECT_SOURCE_DIR}/src ABSOLUTE)

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

# Not a number boxing affects all parts of the project
if(CLX_NAN_BOXING_ACTIVATED)
    add_compile_definitions(NAN_BOXING)
endif()

# We determine the compiler so we can do some optimization for a specific compiler
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_definitions(COMPILER_GCC)
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_compile_definitions(COMPILER_CLANG)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    add_compile_definitions(COMPILER_MSVC)
elseif (CMAKE_C_COMPILER_ID STREQUAL "Intel")
    add_compile_definitions(COMPILER_INTEL)
else()
    add_compile_definitions(COMPILER_UNKNOWN)
endif()

# Builds the cellox interpreter
add_subdirectory(src)

if(CLX_BUILD_TOOLS)
    # Builds the benchmark runner tool for the cellox compiler
    add_subdirectory(benchmark)
    # Builds the disassmbler tool (for cellox chunk files)
    add_subdirectory(disassembler)
endif()

# Please use msvc for testing (building googletest fails using gcc) and specify one as the number of parallel test jobs ("-j 1") when you execute the tests.
if(MSVC AND CLX_BUILD_TESTS)
    # C++ 14 standard is required for the google test framework
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED True)    
    enable_testing()
    # Builds tests for the compiler
    add_subdirectory(test)
endif()
