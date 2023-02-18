#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Else, ClassInElse) {
    test_failing_cellox_program("else_statement/class_in_else.clx", "[line 4] Error at 'class': Expect expression.\n");
}

TEST(Else, Dangling) {
    test_cellox_program("else_statement/dangling.clx", "yes\n");
}

TEST(Else, FunctionInElse) {
    test_failing_cellox_program("else_statement/function_in_else.clx", "[line 4] Error at 'fun': Expect expression.\n");
}

TEST(Else, Simple) {
    test_cellox_program("else_statement/simple.clx", "yes\nyes\nblock\n");
}

TEST(Else, VariableInElse) {
    test_failing_cellox_program("else_statement/variable_in_else.clx", "[line 4] Error at 'var': Expect expression.\n");
}
