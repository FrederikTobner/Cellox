#include "test_cellox.hh"

#include "gtest/gtest.h"

#include "initializer.h"

static void test_program(std::string const & programPath, std::string const & expectedOutput, bool producesError);

void test_cellox_program(std::string const & programPath, std::string const & expectedOutput) {
    test_program(programPath, expectedOutput, false);
}

void test_failing_cellox_program(std::string const & programPath, std::string const & expectedOutput) {
    test_program(programPath, expectedOutput, true);
}

void test_compiled_cellox_program(std::string const & programPath, std::string const & expectedOutput) {
    std::string filePath = TEST_PROGRAM_BASE_PATH;
    filePath.append(programPath);
    initializer_run_from_file(filePath.c_str(), true);
    // We need to alter the path because we want to check whether the compiled version works
    std::string alteredPath = programPath;
    alteredPath.replace(alteredPath.end() - 3, alteredPath.end(), "cxcf");
    test_program(alteredPath, expectedOutput, false);
}

/// @brief Test a cellox program
/// @param programPath The path of the program that is tested
/// @param expectedOutput The expected output of the program
/// @param producesError Determines wheather the rpogram leads to a runtime/compiler error
static void test_program(std::string const & programPath, std::string const & expectedOutput, bool producesError) {
    // Create absolute filepath
    std::string filePath = TEST_PROGRAM_BASE_PATH;
    filePath.append(programPath);

    // Redirect output
    std::string actual_output;
    if (producesError) {
    testing::internal::CaptureStderr();
    } else {
    testing::internal::CaptureStdout();
    }
    // Execute Test 🚀
    initializer_run_from_file(filePath.c_str(), false);

    if (producesError) {
        actual_output = testing::internal::GetCapturedStderr();
    } else {
        actual_output = testing::internal::GetCapturedStdout();
    }

    ASSERT_EQ(expectedOutput, actual_output);
}
