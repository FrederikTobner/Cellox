#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(BinaryOperators, Plus)
{
    test_program("binaryOperators/plusoperator.clx", "8\ntest\n");
}

TEST(BinaryOperators, Minus)
{
    test_program("binaryOperators/minusoperator.clx", "2\n");
}

TEST(BinaryOperators, Multiply)
{
    test_program("binaryOperators/multiplyoperator.clx", "15\n");
}

TEST(BinaryOperators, Divide)
{
    test_program("binaryOperators/divideoperator.clx", "3\n");
}

TEST(BinaryOperators, Modulo)
{
    test_program("binaryOperators/modulooperator.clx", "2\n");
}

TEST(BinaryOperators, Raise)
{
    test_program("binaryOperators/raiseoperator.clx", "32\n");
}

TEST(BinaryOperators, Greater)
{
    test_program("binaryOperators/greater.clx", "true\nfalse\n");
}

TEST(BinaryOperators, GreaterEqual)
{
    test_program("binaryOperators/greaterequal.clx", "true\ntrue\nfalse\n");
}

TEST(BinaryOperators, Smaller)
{
    test_program("binaryOperators/smaller.clx", "true\nfalse\n");
}

TEST(BinaryOperators, SmallerEqual)
{
    test_program("binaryOperators/smallerequal.clx", "true\ntrue\nfalse\n");
}

TEST(BinaryOperators, Equal)
{
    test_program("binaryOperators/equal.clx", "true\ntrue\ntrue\nfalse\nfalse\nfalse\n");
}

TEST(BinaryOperators, NotEqual)
{
    test_program("binaryOperators/notequal.clx", "true\ntrue\ntrue\nfalse\nfalse\nfalse\n");
}