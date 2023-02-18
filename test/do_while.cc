#include "test_cellox.hh"

#include <gtest/gtest.h>

TEST(DoWhileLoops, AtLeastOnce) {
    test_cellox_program("do_while/at_least_once.clx", "0\n");
}

TEST(DoWhileLoops, BlockBody) {
    test_cellox_program("do_while/block_body.clx", "0\n1\n2\n");
}

TEST(DoWhileLoops, ClassInBody) {
    test_failing_cellox_program("do_while/class_in_body.clx", "[line 2] Error at 'class': Expect expression.\n");
}

TEST(DoWhileLoops, EmptyCondition) {
    test_failing_cellox_program("do_while/empty_condition.clx", "[line 2] Error at ')': Expect expression.\n");
}

TEST(DoWhileLoops, SingleStatementBody) {
    test_cellox_program("do_while/single_statement_body.clx", "0\n1\n2\n");
}

TEST(DoWhileLoops, VarInBody) {
    test_failing_cellox_program("do_while/var_in_body.clx", "[line 2] Error at 'var': Expect expression.\n[line 3] Error at ';': Expect expression.\n");
}