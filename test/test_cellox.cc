#include "test_cellox.hh"

#include "gtest/gtest.h"

#include "init.h"

static void test_program(std::string const & programPath, std::string const & expectedOutput, bool producesError);

void test_cellox_program(std::string const & programPath, std::string const & expectedOutput)
{
    test_program(programPath, expectedOutput, false);
}

void test_failing_cellox_program(std::string const & programPath, std::string const & expectedOutput)
{
    test_program(programPath, expectedOutput, true);
}

static void test_program(std::string const & programPath, std::string const & expectedOutput, bool producesError)
{
    // Create absolute filepath
    std::string filePath = TEST_PROGRAM_BASE_PATH;
    filePath.append(programPath);    
    // Create call args
    char const * args[2];
    *(args + 1) = filePath.c_str();
    
    /* Redirect output */
    char actual_output [16384];
    for (size_t i = 0; i < 16384; i++)
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

    if(producesError)
    {
        // Reset stderr redirection
        #ifdef _WIN32
        freopen("CON", "w", stderr);
        #endif
        #ifdef linux
        freopen("CON", "w", stderr);
        #endif
    }
    else
    {
        // Reset stdout redirection
        #ifdef _WIN32
        freopen("CON", "w", stdout);
        #endif
        #ifdef linux
        freopen("CON", "w", stdout);
        #endif
    }

    ASSERT_STREQ(expectedOutput.c_str(), actual_output);
}