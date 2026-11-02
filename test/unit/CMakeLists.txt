set(UNIT_TESTS ${PROJECT_NAME}UnitTests)

file(GLOB_RECURSE UNIT_TEST_SOURCE_FILES CONFIGURE_DEPENDS *.cc)
file(GLOB_RECURSE UNIT_TEST_HEADER_FILES CONFIGURE_DEPENDS *.h)

add_executable(${UNIT_TESTS} ${UNIT_TEST_SOURCE_FILES} ${UNIT_TEST_HEADER_FILES})

# The source path of the compiler allows shorter includes, the current directory allows including the fixtures
target_include_directories(${UNIT_TESTS} PRIVATE ${SOURCEPATH} ${CMAKE_CURRENT_SOURCE_DIR})

# Location of the standard library that is checked by the layout tests
target_compile_definitions(${UNIT_TESTS} PRIVATE STDLIB_SOURCE_DIR="${PROJECT_SOURCE_DIR}/stdlib/")

target_link_libraries(${UNIT_TESTS} PRIVATE clx_driver GTest::gtest_main)

gtest_discover_tests(${UNIT_TESTS} PROPERTIES LABELS "unit")
