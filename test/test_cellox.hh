#ifndef TEST_CELLOX
#define TEST_CELLOX

#include <stdbool.h>
#include <string>

void test_cellox_program(std::string const & programPath, std::string const & expectedOutput, bool producesError);

#endif