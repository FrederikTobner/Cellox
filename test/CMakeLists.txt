cmake_minimum_required(VERSION 3.16)

#C++ 14 needed for Google Test Framework
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(THIS CelloxTests)

#Fetches google-test framework from the github repo
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

set(SOURCEPATH ../src/)

# testfiles
set(TESTSOURCES
assignment_operator.cc
binary_operator.cc
comments.cc
escape_sequences.cc
for_loops.cc
functions.cc
index_operator.cc
literal_expressions.cc
logical_operators.cc
test_cellox.cc
test_cellox.hh
unary_operator.cc
)

# source files of the interpreter
set(CELLOXSOURCES
${SOURCEPATH}chunk.c
${SOURCEPATH}compiler.c
${SOURCEPATH}compiler.h
${SOURCEPATH}dynamic_array.c
${SOURCEPATH}debug.c
${SOURCEPATH}init.c 
${SOURCEPATH}lexer.c
${SOURCEPATH}memory.c 
${SOURCEPATH}native_fun.c
${SOURCEPATH}object.c
${SOURCEPATH}string_utils.c
${SOURCEPATH}table.c
${SOURCEPATH}value.c  
${SOURCEPATH}virtual_machine.c
)

# headers of the interpreter
set(CELLOXHEADERS
${SOURCEPATH}chunk.h
${SOURCEPATH}common.h
${SOURCEPATH}compiler.h
${SOURCEPATH}dynamic_array.h
${SOURCEPATH}debug.h
${SOURCEPATH}init.h
${SOURCEPATH}lexer.h
${SOURCEPATH}memory.h
${SOURCEPATH}native_fun.h
${SOURCEPATH}object.h
${SOURCEPATH}string_utils.h
${SOURCEPATH}table.h
${SOURCEPATH}value.h
${SOURCEPATH}virtual_machine.h
)

# Flag for testing
add_compile_definitions(CELLOX_TEST)

add_executable(${THIS} ${TESTSOURCES} ${CELLOXSOURCES} ${CELLOXHEADERS})

target_include_directories(${THIS} PUBLIC ${SOURCEPATH})

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

#Fetches google test
FetchContent_MakeAvailable(googletest)

#Links googletest libary and cellox tests
target_link_libraries(
  ${THIS}
  GTest::gtest_main
)
include(googletest)

# Automatic discovering of tests
gtest_discover_tests(${THIS})