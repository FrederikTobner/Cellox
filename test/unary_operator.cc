#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(UnaryOperator, Negate)
{
    test_program("unaryOperators/negate.clx", "-5\n10\n");
}

TEST(UnaryOperator, Not)
{
    test_program("unaryOperators/not.clx", "false\ntrue\n");
}