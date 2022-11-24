cmake_minimum_required(VERSION 3.24)

#C++ 14 needed for Google Test Framework
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(THIS ${PROJECT_NAME}Tests)

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

# Precompiles common.h to speed up compilation of the target
if( MSVC )    
    target_precompile_headers(${PROJECT_NAME} PUBLIC ${SOURCEPATH}common.h ${SOURCEPATH}common.c) # VisualStudio only accepts header files that also have a source file
else()
    target_precompile_headers(${PROJECT_NAME} PUBLIC ${SOURCEPATH}common.h)
endif()

# Builds cellox tests
add_executable(${THIS} ${TESTSOURCES} ${CELLOXSOURCES} ${CELLOXHEADERS})

# Includes SourcePath of the compiler for shorter includes
target_include_directories(${THIS} PUBLIC ${SOURCEPATH})

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

#Fetches google test
FetchContent_MakeAvailable(googletest)

#Links googletest libary and cellox tests
target_link_libraries(${THIS} GTest::gtest_main)

# Includes google test framework
include(googletest)
# Automatic discovering of tests
gtest_discover_tests(${THIS})