#include "test_cellox.hh"

#include "gtest/gtest.h"

#include "init.h"

void test_cellox_program(std::string const & programPath, std::string const & expectedOutput, bool producesError)
{
    // Create absolute filepath
    std::string filePath = TEST_PROGRAM_BASE_PATH;
    filePath.append(programPath);    
    // Create call args
    char const * args[2];
    *(args + 1) = filePath.c_str();
    
    /* Redirect output 
     * TODO fix possible buffer overflow in tests
     */
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
    
    // Execute Test 🚀
    init_initialize(2, args);
    ASSERT_STREQ(expectedOutput.c_str(), actual_output);
}