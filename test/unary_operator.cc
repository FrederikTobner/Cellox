#include <gtest/gtest.h>
#include "test_cellox.hh"

extern "C"
{
#include "init.h"
}

TEST(UnaryOperator, Negate)
{
    test_cellox_program("unaryOperators/negate.clx", "-5\n10\n");
}

TEST(UnaryOperator, Not)
{
    test_cellox_program("unaryOperators/not.clx", "false\ntrue\n");
}