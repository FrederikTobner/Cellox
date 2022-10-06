#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(BinaryOperators, Plus)
{
    test_cellox_program("binaryOperators/plusoperator.clx", "8\ntest\n", false);
}

TEST(BinaryOperators, Minus)
{
    test_cellox_program("binaryOperators/minusoperator.clx", "2\n", false);
}

TEST(BinaryOperators, Multiply)
{
    test_cellox_program("binaryOperators/multiplyoperator.clx", "15\n", false);
}

TEST(BinaryOperators, Divide)
{
    test_cellox_program("binaryOperators/divideoperator.clx", "3\n", false);
}

TEST(BinaryOperators, Modulo)
{
    test_cellox_program("binaryOperators/modulooperator.clx", "2\n", false);
}

TEST(BinaryOperators, Raise)
{
    test_cellox_program("binaryOperators/raiseoperator.clx", "32\n", false);
}

TEST(BinaryOperators, Greater)
{
    test_cellox_program("binaryOperators/greater.clx", "true\nfalse\n", false);
}

TEST(BinaryOperators, GreaterEqual)
{
    test_cellox_program("binaryOperators/greaterequal.clx", "true\ntrue\nfalse\n", false);
}

TEST(BinaryOperators, Smaller)
{
    test_cellox_program("binaryOperators/smaller.clx", "true\nfalse\n", false);
}

TEST(BinaryOperators, SmallerEqual)
{
    test_cellox_program("binaryOperators/smallerequal.clx", "true\ntrue\nfalse\n", false);
}

TEST(BinaryOperators, Equal)
{
    test_cellox_program("binaryOperators/equal.clx", "true\ntrue\ntrue\nfalse\nfalse\nfalse\n", false);
}

TEST(BinaryOperators, NotEqual)
{
    test_cellox_program("binaryOperators/notequal.clx", "true\ntrue\ntrue\nfalse\nfalse\nfalse\n", false);
}