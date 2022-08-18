#include "testUtils.hh"
#include <gtest/gtest.h>

extern "C"
{
#include "init.h"
}

void test_program(char *programPath, const char *expectedOutput)
{
    char *actual_output = (char *)calloc(1024, sizeof(char));
#ifdef _WIN32
    freopen("NUL", "a", stdout);
#endif
#ifdef linux
    freopen("/dev/nul", "a", stdout);
#endif

    setbuf(stdout, actual_output);
    char *filePath = (char *)calloc(1024, sizeof(char));
    strcat(filePath, TEST_PROGRAM_BASE_PATH);
    strcat(filePath, programPath);
    const char *args[4];
    args[1] = filePath;
    init(2, args);
    ASSERT_STREQ(expectedOutput, actual_output);
    free(actual_output);
    free(filePath);
}