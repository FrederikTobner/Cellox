#ifndef TEST_CELLOX
#define TEST_CELLOX
#include <stdbool.h>

#define test_program(path, expectedOutput) test_cellox_program(path, expectedOutput, false);

#define test_failing_program(path, expectedOutput) test_cellox_program(path, expectedOutput, true);

void test_cellox_program(char const * programPath, char const * expectedOutput, bool producesError);

#endif