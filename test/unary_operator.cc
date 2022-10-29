#include "test_cellox.hh"

#include <gtest/gtest.h>

TEST(UnaryOperator, Negate)
{
    test_cellox_program("unaryOperators/negate.clx", "-5\n10\n");
}

TEST(UnaryOperator, Not)
{
    test_cellox_program("unaryOperators/not.clx", "false\ntrue\n");
}