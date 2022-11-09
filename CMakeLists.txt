cmake_minimum_required(VERSION 3.16)
project(Cellox
VERSION 0.1.0
)

# C99 standard is required to build the interpreter
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Path of the sources of the cellox interpreter
set(SOURCEPATH ../src/)

# source files of the interpreter
set(CELLOXSOURCES
${SOURCEPATH}chunk.c
${SOURCEPATH}compiler.c
${SOURCEPATH}compiler.h
${SOURCEPATH}dynamic_value_array.c
${SOURCEPATH}debug.c
${SOURCEPATH}init.c
${SOURCEPATH}lexer.c
${SOURCEPATH}memory.c
${SOURCEPATH}native_functions.c
${SOURCEPATH}object.c
${SOURCEPATH}string_utils.c
${SOURCEPATH}table.c
${SOURCEPATH}value.c
${SOURCEPATH}virtual_machine.c
)

# headers of the interpreter
set(CELLOXHEADERS
${SOURCEPATH}chunk.h
${SOURCEPATH}compiler.h
${SOURCEPATH}dynamic_value_array.h
${SOURCEPATH}debug.h
${SOURCEPATH}init.h
${SOURCEPATH}lexer.h
${SOURCEPATH}memory.h
${SOURCEPATH}native_functions.h
${SOURCEPATH}object.h
${SOURCEPATH}string_utils.h
${SOURCEPATH}table.h
${SOURCEPATH}value.h
${SOURCEPATH}virtual_machine.h
)

# Links compiler with the math.h libary
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-lm)
endif()

# Builds the cellox interpreter
add_subdirectory(src)

# Builds benchmarks for the cellox interpreter
add_subdirectory(benchmark)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
# usual approach doesn't work under linux using gcc 
else()
# Please use msvc x86 for testing (pthreads isn't included under windows using mingw and gcc and include googletest fails under ubuntu and tests behave weird under x64)
enable_testing()
# Builds tests for the interpreter
add_subdirectory(test)
endif()