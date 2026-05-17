#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(BinaryOperators, Divide) {
    test_cellox_program("binary_operators/divide.clx", "3\n");
}

TEST(BinaryOperators, Equal) {
    test_cellox_program("binary_operators/equal.clx", "true\ntrue\ntrue\nfalse\nfalse\nfalse\n");
}

TEST(BinaryOperators, Greater) {
    test_cellox_program("binary_operators/greater.clx", "true\nfalse\n");
}

TEST(BinaryOperators, GreaterEqual) {
    test_cellox_program("binary_operators/greater_equal.clx", "true\ntrue\nfalse\n");
}

TEST(BinaryOperators, minus) {
    test_cellox_program("binary_operators/minus.clx", "2\n");
}

TEST(BinaryOperators, modulo) {
    test_cellox_program("binary_operators/modulo.clx", "2\n");
}

TEST(BinaryOperators, multiply) {
    test_cellox_program("binary_operators/multiply.clx", "15\n");
}

TEST(BinaryOperators, not_equal) {
    test_cellox_program("binary_operators/not_equal.clx", "true\ntrue\ntrue\nfalse\nfalse\nfalse\n");
}

TEST(BinaryOperators, plus) {
    test_cellox_program("binary_operators/plus.clx", "8\ntest\n");
}

TEST(BinaryOperators, raise) {
    test_cellox_program("binary_operators/raise.clx", "32\n");
}

TEST(BinaryOperators, smaller) {
    test_cellox_program("binary_operators/smaller.clx", "true\nfalse\n");
}

TEST(BinaryOperators, smaller_equal) {
    test_cellox_program("binary_operators/smaller_equal.clx", "true\ntrue\nfalse\n");
}

TEST(BinaryOperators, minusTypeError) {
    test_failing_cellox_program("binary_operators/minus_type_error.clx",
                                "Operands must be numbers but they are a string object and a string object\n[line "
                                "1] in script\n");
}

TEST(BinaryOperators, plusNullTypeError) {
    test_failing_cellox_program(
        "binary_operators/plus_null_type_error.clx",
        "Operands must be two numbers, two strings, an array and a value or an array and an array, but they are a "
        "numerical value and a undefiened value\n[line 1] in script\n");
}
