#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(If, ClassInThen)
{
    test_failing_cellox_program("if_statement/class_in_then.clx", "[line 1] Error at 'class': Expect expression.\n");
}

TEST(If, FunctionInThen)
{
    test_failing_cellox_program("if_statement/function_in_then.clx", "[line 1] Error at 'fun': Expect expression.\n");
}

TEST(If, Simple)
{
    test_cellox_program("if_statement/simple.clx", "yes\nblock\ntrue\n");
}

TEST(If, VariableInThen)
{
    test_failing_cellox_program("if_statement/variable_in_then.clx", "[line 1] Error at 'var': Expect expression.\n");
}
