#include <gtest/gtest.h>
#include "TestUtils.hh"

extern "C"
{
#include "init.h"
}

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