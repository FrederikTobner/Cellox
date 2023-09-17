cmake_minimum_required(VERSION 3.24)

set(INTERPRETER_TESTS ${PROJECT_NAME}Tests)

#Fetches google-test framework from the github repo
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

file(GLOB_RECURSE TEST_SOURCE_FILES *.cc *.c *.cpp)
file(GLOB_RECURSE TEST_HEADER_FILES *.hh *.h *.hpp)

add_compile_definitions(
# Flag that indicates that test are executed
CELLOX_TESTS_RUNNING
# Directory of the cellox e2e tests
TEST_PROGRAM_BASE_PATH="${PROJECT_SOURCE_DIR}/test/"
)

# Builds cellox tests
add_executable(${INTERPRETER_TESTS} 
${CELLOX_SOURCE_FILES} ${CELLOX_HEADER_FILES} 
${TEST_SOURCE_FILES} ${TEST_HEADER_FILES})

# Includes SourcePath of the compiler for shorter includes and the config file
target_include_directories(${INTERPRETER_TESTS} PUBLIC ${SOURCEPATH} ${PROJECT_BINARY_DIR}/src)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

#Fetches google test
FetchContent_MakeAvailable(googletest)

#Links googletest libary and cellox tests
target_link_libraries(${INTERPRETER_TESTS} GTest::gtest_main)

# Includes google test framework specific macros
include(GoogleTest)
# Automatic discovering of tests
gtest_discover_tests(${INTERPRETER_TESTS})
