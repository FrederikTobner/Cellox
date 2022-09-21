#include <gtest/gtest.h>

#include "test_cellox.hh"

extern "C"
{
#include "init.h"
}

void test_cellox_program(char const *programPath, char const *expectedOutput)
{
    char *actual_output = (char *)calloc(4096, sizeof(char));
    if(!actual_output)
        return;
#ifdef _WIN32
    freopen("NUL", "a", stdout);
#endif
#ifdef linux
    freopen("/dev/nul", "a", stdout);
#endif
    setbuf(stdout, actual_output);
    char *filePath = (char *)malloc(sizeof(char) * 1024);
    if(!filePath)
        return;
    *filePath = '\0';
    strcat(filePath, TEST_PROGRAM_BASE_PATH);
    strcat(filePath, programPath);
    const char *args[2];
    args[1] = filePath;
    init_initialize(2, args);
    ASSERT_STREQ(expectedOutput, actual_output);
    free(actual_output);
    free(filePath);
}