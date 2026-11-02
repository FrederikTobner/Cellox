cmake_minimum_required(VERSION 3.16)

project(Cellox 
VERSION 0.1.0 
LANGUAGES C CXX
)

# C99 standard is required to build the compiler
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Optional NAN-Boxing (Not a number boxing / Not a number tagging)
option(CLX_NAN_BOXING_ACTIVATED "Determines whether \"not a number boxing / tagging\" is used" ON)

# Debug options (only have an effect on debug builds)
option(CLX_DEBUG_PRINT_BYTECODE "Determines whether the chunks are dissassembled and the bytecode is printed" OFF)
option(CLX_DEBUG_TRACE_EXECUTION "Determines whether the execution shall be traced" OFF)
option(CLX_DEBUG_STRESS_GARBAGE_COLLECTOR "Determines whether the garbage collector shall be stressed" OFF)
option(CLX_DEBUG_LOG_GARBAGE_COLLECTION "Determines whether the garbage collection be logged" OFF)

# Optimization diagnostics (all builds)
option(CLX_OPTIMIZATION_PRINT_STATS "Print optimization summary statistics for each compiled chunk" OFF)
option(CLX_OPTIMIZATION_VERBOSE_STATS "Print verbose per-pass optimization diagnostics" OFF)

# Configures the tests (and fetches the google test framework). The test executables are not part of the default build,
# build the target CelloxTests to build them.
option(BUILD_TESTING "Determines whether the tests shall be configured" OFF)
include(CTest)

include(CheckIncludeFile)

get_filename_component(SOURCEPATH ${PROJECT_SOURCE_DIR}/src ABSOLUTE)

# Not a number boxing affects all parts of the project
if(CLX_NAN_BOXING_ACTIVATED)
    add_compile_definitions(NAN_BOXING)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(CelloxHelpers)

set(CLX_CMAKE_CONDITIONALS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/conditionals")
include("${CLX_CMAKE_CONDITIONALS_DIR}/compiler/select.cmake")
include("${CLX_CMAKE_CONDITIONALS_DIR}/os/select.cmake")
include("${CLX_CMAKE_CONDITIONALS_DIR}/build_types.cmake")

clx_os_check_core_headers()

# Standard library of Cellox source files
# Must come before src/ so CELLOX_STDLIB_SOURCE_DIR is set before cellox_config.h is generated.
add_subdirectory(stdlib)

file(GLOB_RECURSE CELLOX_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE CELLOX_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

# Allow includes from the src root (e.g. frontend/... and backend/...)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

LIST(REMOVE_ITEM CELLOX_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/common.c )
LIST(REMOVE_ITEM CELLOX_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/common.h)

# Builds the cellox interpreter
add_subdirectory(src)

LIST(REMOVE_ITEM CELLOX_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c CELLOX_SOURCE_FILES )

# The development tools are not part of the default build, build the target CelloxTools to build them
# Benchmark runner tool for the cellox compiler
add_subdirectory(benchmark EXCLUDE_FROM_ALL)
# Disassmbler tool (for cellox chunk files)
add_subdirectory(disassembler EXCLUDE_FROM_ALL)
add_custom_target(${PROJECT_NAME}Tools DEPENDS ${PROJECT_NAME}Benchmarks ${PROJECT_NAME}Disassembler)

if(BUILD_TESTING)
    add_subdirectory(test EXCLUDE_FROM_ALL)
endif()