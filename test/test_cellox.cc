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
    char actual_output[1024];
    for (size_t i = 0; i < 1024; i++) {
        actual_output[i] = '\0';
    }
    if (producesError) {
#ifdef OS_WINDOWS
        freopen("NUL", "a", stderr);
#elif OS_UNIX_LIKE
        freopen("/dev/nul", "a", stderr);
#endif
        setbuf(stderr, actual_output);
    } else {
#ifdef OS_WINDOWS
        freopen("NUL", "a", stdout);
#elif OS_UNIX_LIKE
        freopen("/dev/nul", "a", stdout);
#endif
        setbuf(stdout, actual_output);
    }

    // Execute Test 🚀
    initializer_run_from_file(filePath.c_str(), false);

    if (producesError) {
        freopen("CON", "w", stderr);
    } else {
        freopen("CON", "w", stdout);
    }

    ASSERT_STREQ(expectedOutput.c_str(), actual_output);
}