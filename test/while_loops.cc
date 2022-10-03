#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(WhileLoops, BlockBody)
{
    test_program("whileLoops/BlockBody.clx", "0\n1\n2\n");
}

TEST(WhileLoops, EmptyCondition)
{
    test_failing_program("whileLoops/EmptyCondition.clx", "[line 1] Error at ')': Expect expression.\n");
}


TEST(WhileLoops, SingleExpressionBody)
{
    test_program("whileLoops/SingleExpressionBody.clx", "1\n2\n3\n");
}