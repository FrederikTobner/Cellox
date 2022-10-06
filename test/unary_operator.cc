#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(UnaryOperator, Negate)
{
    test_cellox_program("unaryOperators/negate.clx", "-5\n10\n", false);
}

TEST(UnaryOperator, Not)
{
    test_cellox_program("unaryOperators/not.clx", "false\ntrue\n", false);
}