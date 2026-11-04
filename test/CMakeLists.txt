cmake_minimum_required(VERSION 3.24)

LIST(REMOVE_ITEM CELLOX_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../src/main.c CELLOX_SOURCE_FILES )
# C++17 is required by the google test framework (only the tests are written in C++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Fetches the google test framework from the github repo
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.18.0
)
FetchContent_MakeAvailable(googletest)

# Includes google test framework specific macros (gtest_discover_tests)
include(GoogleTest)

# Every type of test has its own target (and CMakeLists.txt)
add_subdirectory(unit)
add_subdirectory(integration)
add_subdirectory(e2e)
add_subdirectory(fuzz)

# Builds all test executables
add_custom_target(${PROJECT_NAME}Tests
    DEPENDS ${PROJECT_NAME}UnitTests ${PROJECT_NAME}IntegrationTests ${PROJECT_NAME}E2ETests ${PROJECT_NAME}FuzzTests
)
