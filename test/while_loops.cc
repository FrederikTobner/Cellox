#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(WhileLoops, BlockBody)
{
    test_cellox_program("whileLoops/BlockBody.clx", "0\n1\n2\n", false);
}

TEST(WhileLoops, ClassInBody)
{
    test_cellox_program("whileLoops/classInBody.clx", "[line 2] Error at 'class': Expect expression.\n", true);
}

TEST(WhileLoops, EmptyCondition)
{
    test_cellox_program("whileLoops/emptyCondition.clx", "[line 1] Error at ')': Expect expression.\n", true);
}

TEST(WhileLoops, SingleExpressionBody)
{
    test_cellox_program("whileLoops/SingleExpressionBody.clx", "1\n2\n3\n", false);
}
