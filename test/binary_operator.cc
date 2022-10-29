#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(BinaryOperators, Divide)
{
    test_cellox_program("binaryOperators/divide.clx", "3\n");
}

TEST(BinaryOperators, Equal)
{
    test_cellox_program("binaryOperators/equal.clx", "true\ntrue\ntrue\nfalse\nfalse\nfalse\n");
}

TEST(BinaryOperators, Greater)
{
    test_cellox_program("binaryOperators/greater.clx", "true\nfalse\n");
}

TEST(BinaryOperators, GreaterEqual)
{
    test_cellox_program("binaryOperators/greater_equal.clx", "true\ntrue\nfalse\n");
}

TEST(BinaryOperators, minus)
{
    test_cellox_program("binaryOperators/minus.clx", "2\n");
}

TEST(BinaryOperators, modulo)
{
    test_cellox_program("binaryOperators/modulo.clx", "2\n");
}

TEST(BinaryOperators, multiply)
{
    test_cellox_program("binaryOperators/multiply.clx", "15\n");
}

TEST(BinaryOperators, plus)
{
    test_cellox_program("binaryOperators/plus.clx", "8\ntest\n");
}

TEST(BinaryOperators, raise)
{
    test_cellox_program("binaryOperators/raise.clx", "32\n");
}

TEST(BinaryOperators, smaller)
{
    test_cellox_program("binaryOperators/smaller.clx", "true\nfalse\n");
}

TEST(BinaryOperators, smaller_equal)
{
    test_cellox_program("binaryOperators/smaller_equal.clx", "true\ntrue\nfalse\n");
}


TEST(BinaryOperators, not_equal)
{
    test_cellox_program("binaryOperators/not_equal.clx", "true\ntrue\ntrue\nfalse\nfalse\nfalse\n");
}