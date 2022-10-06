#include <gtest/gtest.h>

#include "test_cellox.hh"
#include "init.h"

void test_cellox_program(char const * programPath, char const * expectedOutput, bool producesError)
{
    // Create absolute filepath
    char filePath [1024];
    filePath[0] = '\0';
    strcat(filePath, TEST_PROGRAM_BASE_PATH);
    strcat(filePath, programPath);
    
    // Create call args
    const char * args[2];
    *(args + 1) = filePath;
    
    // Redirect output
    char actual_output [4096];
    for (size_t i = 0; i < 4096; i++)
        actual_output[i] = '\0';
    if(producesError)
    {
        #ifdef _WIN32
        freopen("NUL", "a", stderr);
        #endif
        #ifdef linux
        freopen("/dev/nul", "a", stderr);
        #endif
        setbuf(stderr, actual_output);
    }
    else
    {
        #ifdef _WIN32
        freopen("NUL", "a", stdout);
        #endif
        #ifdef linux
        freopen("/dev/nul", "a", stdout);
        #endif
        setbuf(stdout, actual_output);
    }
    
    // Execute Test
    init_initialize(2, args);
    ASSERT_STREQ(expectedOutput, actual_output);
}