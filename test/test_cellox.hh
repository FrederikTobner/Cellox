#ifndef CELLOX_TEST_TEST_H_
#define CELLOX_TEST_TEST_H_

#include <stdbool.h>
#include <string>

/// @brief Tests a cellox program
/// @param programPath The path of the program that is tested
/// @param expectedOutput The expected output of the program
void test_cellox_program(std::string const & programPath, std::string const & expectedOutput);

/// @brief Tests a cellox program that triggers a runtime or compiler error
/// @param programPath The path of the program that is tested
/// @param expectedOutput The expected output of the program
void test_failing_cellox_program(std::string const & programPath, std::string const & expectedOutput);

#endif