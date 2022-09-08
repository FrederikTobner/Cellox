#include <gtest/gtest.h>

#include "test_cellox.hh"

extern "C"
{
#include "init.h"
}

TEST(ForLoops, NormalFor)
{
    test_cellox_program("forLoops/normalFor.clx", "1\n2\n3\n4\n5\n");
}

TEST(ForLoops, NoVariable)
{
    test_cellox_program("forLoops/noVariable.clx", "0\n1\n2\n");
}

TEST(ForLoops, NoCounterIncrementExpression)
{
    test_cellox_program("forLoops/noCounterIncrementExpression.clx", "1\n2\n3\n4\n");
}