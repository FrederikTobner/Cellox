set(FUZZ_TESTS ${PROJECT_NAME}FuzzTests)

file(GLOB_RECURSE FUZZ_TEST_SOURCE_FILES CONFIGURE_DEPENDS *.cpp)

add_executable(${FUZZ_TESTS} ${FUZZ_TEST_SOURCE_FILES})

target_include_directories(${FUZZ_TESTS} PRIVATE ${SOURCEPATH})

target_link_libraries(${FUZZ_TESTS} PRIVATE clx_driver GTest::gtest_main)

gtest_discover_tests(${FUZZ_TESTS} PROPERTIES LABELS "fuzz")
