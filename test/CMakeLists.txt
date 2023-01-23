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
"${SOURCEPATH}/byte-code/chunk_file.c"
"${SOURCEPATH}/middle-end/chunk_optimizer.c"
"${SOURCEPATH}/byte-code/chunk.c"
"${SOURCEPATH}/frontend/compiler.c"
"${SOURCEPATH}/language-models/data-structures/dynamic_value_array.c"
"${SOURCEPATH}//byte-code/chunk_disassembler.c"
"${SOURCEPATH}/backend/garbage_collector.c"
"${SOURCEPATH}/language-models/data-structures/value_hash_table.c"
"${SOURCEPATH}/initializer.c"
"${SOURCEPATH}/frontend/lexer.c"
"${SOURCEPATH}/backend/memory_mutator.c"
"${SOURCEPATH}/backend/native_functions.c"
"${SOURCEPATH}/language-models/object.c"
"${SOURCEPATH}/string_utils.c"
"${SOURCEPATH}/language-models/value.c"
"${SOURCEPATH}/backend/virtual_machine.c"
)
set(TEST_DEPENDENCIES_HEADER_FILES
"${SOURCEPATH}/byte-code/chunk_file.h"
"${SOURCEPATH}/middle-end/chunk_optimizer.h"
"${SOURCEPATH}/byte-code/chunk.h"
"${SOURCEPATH}/frontend/compiler.h"
"${SOURCEPATH}/language-models/data-structures/dynamic_value_array.h"
"${SOURCEPATH}//byte-code/chunk_disassembler.h"
"${SOURCEPATH}/backend/garbage_collector.h"
"${SOURCEPATH}/language-models/data-structures/value_hash_table.h"
"${SOURCEPATH}/initializer.h"
"${SOURCEPATH}/frontend/lexer.h"
"${SOURCEPATH}/backend/memory_mutator.h"
"${SOURCEPATH}/backend/native_functions.h"
"${SOURCEPATH}/language-models/object.h"
"${SOURCEPATH}/string_utils.h"
"${SOURCEPATH}/language-models/value.h"
"${SOURCEPATH}/backend/virtual_machine.h"
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