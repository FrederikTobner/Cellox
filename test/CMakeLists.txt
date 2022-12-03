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
set(TESTSOURCES
assignment_operator.cc
binary_operator.cc
class.cc
comments.cc
else_statement.cc
escape_sequences.cc
fields.cc
for_loops.cc
functions.cc
if_statement.cc
index_operator.cc
limits.cc
literal_expressions.cc
logical_operators.cc
method.cc
native_functions.cc
super.cc
test_cellox.cc
test_cellox.hh
this.cc
unary_operator.cc
variable.cc
while_loops.cc
)

add_compile_definitions(
# Flag that indicates that test are executed
CELLOX_TESTS_RUNNING
# Directory of the cellox integration tests
TEST_PROGRAM_BASE_PATH="${PROJECT_SOURCE_DIR}/test/"
)

if(NAN_BOXING_ACTIVATED)
  add_compile_definitions(NAN_BOXING)
endif()

# Builds cellox tests
add_executable(${INTERPRETER_TESTS} ${TESTSOURCES} ${CELLOXSOURCES} ${CELLOXHEADERS})

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