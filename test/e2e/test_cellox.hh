#ifndef CELLOX_TEST_TEST_H_
#define CELLOX_TEST_TEST_H_

#include <stdbool.h>
#include <string>

/// @brief Tests a cellox program
/// @param programPath The path of the program that is tested
/// @param expectedOutput The expected output of the program
/// @details For testing a cellox program the standard output is redirected to a string so we can do assertions with
/// that given string
void test_cellox_program(std::string const & programPath, std::string const & expectedOutput);

/// @brief Compiles a cellox program, stores it in a file and then executes this file
/// @param programPath The path of the program that is compiled
/// @param expectedOutput Expected output for of the compiled cellox program
/// @details The file that contains the intermediate representation of the program is called a chunk file
void test_compiled_cellox_program(std::string const & programPath, std::string const & expectedOutput);

/// @brief Tests a cellox program that triggers an error at compile- or runtime
/// @param programPath The path of the program that is tested
/// @param expectedOutput The expected output of the program
/// @details For testing a cellox program the standard output is redirected to a string so we can do assertions with
/// that given string
void test_failing_cellox_program(std::string const & programPath, std::string const & expectedOutput);

#endif