cmake_minimum_required(VERSION 3.24)

#C++ 14 needed for Google Test Framework
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(INTERPRETER_TESTS ${PROJECT_NAME}Tests)

#Fetches google-test framework from the github repo
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

# testfiles
set(TEST_SOURCE_FILES
"array.cc"
"assignment_operators.cc"
"binary_operators.cc"
"class.cc"
"comments.cc"
"compile.cc"
"do_while.cc"
"else_statement.cc"
"escape_sequences.cc"
"fields.cc"
"for_loops.cc"
"functions.cc"
"if_statement.cc"
"index_operator.cc"
"limits.cc"
"literal_expressions.cc"
"logical_operators.cc"
"method.cc"
"native_functions.cc"
"range_operator.cc"
"return.cc"
"slice.cc"
"strings.cc"
"super.cc"
"test_cellox.cc"
"this.cc"
"unary_operator.cc"
"variable.cc"
"while.cc"
)

set(TEST_HEADER_FILES
"test_cellox.hh"
)

# dependencies from the interpreter needed to build the tests
set(TEST_DEPENDENCIES_SOURCE_FILES
"${SOURCEPATH}/chunk_file.c"
"${SOURCEPATH}/chunk_optimizer.c"
"${SOURCEPATH}/chunk.c"
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
set(TEST_DEPENDENCIES_HEADER_FILES
"${SOURCEPATH}/chunk_file.h"
"${SOURCEPATH}/chunk_optimizer.h"
"${SOURCEPATH}/chunk.h"
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

add_compile_definitions(
# Flag that indicates that test are executed
CELLOX_TESTS_RUNNING
# Directory of the cellox integration tests
TEST_PROGRAM_BASE_PATH="${PROJECT_SOURCE_DIR}/test/"
)

# Builds cellox tests
add_executable(${INTERPRETER_TESTS} 
${TEST_DEPENDENCIES_SOURCE_FILES} ${TEST_DEPENDENCIES_HEADER_FILES} 
${TEST_SOURCE_FILES} ${TEST_HEADER_FILES})

# Includes SourcePath of the compiler for shorter includes and the config file
target_include_directories(${INTERPRETER_TESTS} PUBLIC ${SOURCEPATH} ${PROJECT_BINARY_DIR}/src)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

#Fetches google test
FetchContent_MakeAvailable(googletest)

#Links googletest libary and cellox tests
target_link_libraries(${INTERPRETER_TESTS} GTest::gtest_main)

# Includes google test framework
include(googletest)
# Automatic discovering of tests
gtest_discover_tests(${INTERPRETER_TESTS})