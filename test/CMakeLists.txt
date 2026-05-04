cmake_minimum_required(VERSION 3.24)

set(E2E_TESTS ${PROJECT_NAME}E2ETests)
set(INTEGRATION_TESTS ${PROJECT_NAME}IntegrationTests)
set(UNIT_TESTS ${PROJECT_NAME}UnitTests)
set(FUZZ_TESTS ${PROJECT_NAME}FuzzTests)

#Fetches google-test framework from the github repo
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

file(GLOB_RECURSE E2E_TEST_SOURCE_FILES e2e/*.cc e2e/*.c e2e/*.cpp)
file(GLOB_RECURSE E2E_TEST_HEADER_FILES e2e/*.hh e2e/*.h e2e/*.hpp)
file(GLOB_RECURSE INTEGRATION_TEST_SOURCE_FILES integration/*.cc integration/*.c integration/*.cpp)
file(GLOB_RECURSE INTEGRATION_TEST_HEADER_FILES integration/*.hh integration/*.h integration/*.hpp)
file(GLOB_RECURSE UNIT_TEST_SOURCE_FILES unit/*.cc unit/*.c unit/*.cpp)
file(GLOB_RECURSE UNIT_TEST_HEADER_FILES unit/*.hh unit/*.h unit/*.hpp)

set(FUZZ_TEST_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/unit/property_fuzz_unit.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/integration/property_fuzz_serialization_integration.cc
)

list(REMOVE_ITEM UNIT_TEST_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/unit/property_fuzz_unit.cc)
list(REMOVE_ITEM INTEGRATION_TEST_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/integration/property_fuzz_serialization_integration.cc)

add_compile_definitions(
# Flag that indicates that test are executed
CELLOX_TESTS_RUNNING
# Directory of the cellox e2e tests
TEST_PROGRAM_BASE_PATH="${PROJECT_SOURCE_DIR}/test/"
)

add_executable(${E2E_TESTS}
    ${E2E_TEST_SOURCE_FILES} ${E2E_TEST_HEADER_FILES})

add_executable(${INTEGRATION_TESTS}
    ${INTEGRATION_TEST_SOURCE_FILES} ${INTEGRATION_TEST_HEADER_FILES})

add_executable(${UNIT_TESTS}
    ${UNIT_TEST_SOURCE_FILES} ${UNIT_TEST_HEADER_FILES})

add_executable(${FUZZ_TESTS}
    ${FUZZ_TEST_SOURCE_FILES})

# Includes SourcePath of the compiler for shorter includes and top-level test include paths
target_include_directories(${E2E_TESTS} PUBLIC ${SOURCEPATH} ${CMAKE_CURRENT_SOURCE_DIR}/e2e)
target_include_directories(${INTEGRATION_TESTS} PUBLIC ${SOURCEPATH})
target_include_directories(${UNIT_TESTS} PUBLIC ${SOURCEPATH})
target_include_directories(${FUZZ_TESTS} PUBLIC ${SOURCEPATH})

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

#Fetches google test
FetchContent_MakeAvailable(googletest)

# Links cellox libraries and googletest
target_link_libraries(${E2E_TESTS} PRIVATE clx_driver GTest::gtest_main)
target_link_libraries(${INTEGRATION_TESTS} PRIVATE clx_driver GTest::gtest_main)
target_link_libraries(${UNIT_TESTS} PRIVATE clx_driver GTest::gtest_main)
target_link_libraries(${FUZZ_TESTS} PRIVATE clx_driver GTest::gtest_main)

# Includes google test framework specific macros
include(GoogleTest)
# Automatic discovery and labeling of tests
gtest_discover_tests(${E2E_TESTS} TEST_LIST E2E_TEST_LIST PROPERTIES LABELS "e2e")
gtest_discover_tests(${INTEGRATION_TESTS} PROPERTIES LABELS "integration")
gtest_discover_tests(${UNIT_TESTS} PROPERTIES LABELS "unit")
gtest_discover_tests(${FUZZ_TESTS} PROPERTIES LABELS "fuzz")

# Optimization pass unit tests (standalone, no googletest dependency)
add_subdirectory(middle-end)
