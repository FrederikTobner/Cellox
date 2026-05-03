#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Functions, duplicateParameter) {
    test_failing_cellox_program("functions/duplicate_parameter.clx",
                                "[line 1] Error at 'a': Already a variable with this name in this scope.\n");
}

TEST(Functions, emptyBody) {
    test_cellox_program("functions/empty_body.clx", "null\n");
}

TEST(Functions, nestedFunction) {
    test_cellox_program("functions/nested_function.clx", "0\n1\n1\n2\n3\n5\n8\n13\n21\n34\n");
}

TEST(Functions, parameters) {
    test_cellox_program("functions/parameters.clx", "0\n1\n3\n6\n10\n15\n21\n28\n36\n");
}

TEST(Functions, recursion) {
    test_cellox_program("functions/recursion.clx", "21\n");
}