#ifndef CELLOX_TEST_TEST_H_
#define CELLOX_TEST_TEST_H_

#include <stdbool.h>
#include <string>

void test_cellox_program(std::string const & programPath, std::string const & expectedOutput);

void test_failing_cellox_program(std::string const & programPath, std::string const & expectedOutput);

#endif