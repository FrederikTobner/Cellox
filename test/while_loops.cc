#include "test_cellox.hh"

#include <gtest/gtest.h>

TEST(WhileLoops, BlockBody)
{
    test_cellox_program("while_loops/block_body.clx", "0\n1\n2\n");
}

TEST(WhileLoops, ClassInBody)
{
    test_failing_cellox_program("while_loops/class_in_body.clx", "[line 2] Error at 'class': Expect expression.\n");
}

TEST(WhileLoops, EmptyCondition)
{
    test_failing_cellox_program("while_loops/empty_condition.clx", "[line 1] Error at ')': Expect expression.\n");
}

TEST(WhileLoops, SingleExpressionBody)
{
    test_cellox_program("while_loops/single_expression_body.clx", "1\n2\n3\n");
}

TEST(WhileLoops, VarInBody)
{
    test_failing_cellox_program("while_loops/var_in_body.clx", "[line 2] Error at 'var': Expect expression.\n");
}
