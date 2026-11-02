set(INTEGRATION_TESTS ${PROJECT_NAME}IntegrationTests)

file(GLOB_RECURSE INTEGRATION_TEST_SOURCE_FILES CONFIGURE_DEPENDS *.cc)
file(GLOB_RECURSE INTEGRATION_TEST_HEADER_FILES CONFIGURE_DEPENDS *.h)

add_executable(${INTEGRATION_TESTS} ${INTEGRATION_TEST_SOURCE_FILES} ${INTEGRATION_TEST_HEADER_FILES})

# The source path of the compiler allows shorter includes, the current directory allows including the fixtures
target_include_directories(${INTEGRATION_TESTS} PRIVATE ${SOURCEPATH} ${CMAKE_CURRENT_SOURCE_DIR})

# Location of the standard library that is loaded by the module loading tests
target_compile_definitions(${INTEGRATION_TESTS} PRIVATE STDLIB_SOURCE_DIR="${PROJECT_SOURCE_DIR}/stdlib/")

target_link_libraries(${INTEGRATION_TESTS} PRIVATE clx_driver GTest::gtest_main)

gtest_discover_tests(${INTEGRATION_TESTS} PROPERTIES LABELS "integration")
