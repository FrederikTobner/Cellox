#include <gtest/gtest.h>

#include <cstdlib>
#include <string>

#include "base/common.h"
#include "driver/initializer.h"

static int run_program(std::string const & programPath, bool compile = false) {
    std::string filePath = std::string(TEST_PROGRAM_BASE_PATH) + programPath;
    testing::internal::CaptureStderr();
    int exitCode = initializer_run_from_file(filePath.c_str(), compile);
    testing::internal::GetCapturedStderr();
    return exitCode;
}

TEST(ExitCodes, Success) {
    testing::internal::CaptureStdout();
    int exitCode = run_program("variable/uninitialized.clx");
    testing::internal::GetCapturedStdout();
    ASSERT_EQ(EXIT_SUCCESS, exitCode);
}

TEST(ExitCodes, CompilationError) {
    ASSERT_EQ(EXIT_CODE_COMPILATION_ERROR, run_program("while/empty_condition.clx"));
}

TEST(ExitCodes, RuntimeError) {
    ASSERT_EQ(EXIT_CODE_RUNTIME_ERROR, run_program("variable/undefined.clx"));
}

TEST(ExitCodes, MissingSourceFile) {
    ASSERT_EQ(EXIT_CODE_INPUT_OUTPUT_ERROR, run_program("does_not_exist.clx"));
}

TEST(ExitCodes, MissingChunkFile) {
    ASSERT_EQ(EXIT_CODE_INPUT_OUTPUT_ERROR, run_program("does_not_exist.cxcf"));
}

TEST(ExitCodes, UnsupportedFileType) {
    ASSERT_EQ(EXIT_CODE_INPUT_OUTPUT_ERROR, run_program("does_not_exist.txt"));
}

TEST(ExitCodes, CompilingAChunkFile) {
    ASSERT_EQ(EXIT_CODE_INPUT_OUTPUT_ERROR, run_program("does_not_exist.cxcf", true));
}
