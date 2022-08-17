#include <gtest/gtest.h>
#include "TestUtils.hh"

extern "C"
{
#include "init.h"
}

TEST(UnaryOperator, Negate)
{
    test_program("unaryOperators/negate.clx", "-5\n10\n");
}

TEST(UnaryOperator, Not)
{
    test_program("unaryOperators/not.clx", "false\ntrue\n");
}