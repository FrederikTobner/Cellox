#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(BreakStatement, WhileBreak) {
    test_cellox_program("break/while_break.clx", "0\n1\n2\n");
}

TEST(BreakStatement, ForBreak) {
    test_cellox_program("break/for_break.clx", "0\n1\n2\n");
}

TEST(BreakStatement, DoWhileBreak) {
    test_cellox_program("break/do_while_break.clx", "0\n1\n2\n");
}

TEST(BreakStatement, NestedBreakOnlyExitsInnerLoop) {
    test_cellox_program("break/nested_break.clx", "0,0\n1,0\n2,0\n");
}

TEST(BreakStatement, BreakOutsideLoop) {
    test_failing_cellox_program("break/break_outside_loop.clx",
                                "[line 1] Error at 'break': Can't use 'break' outside of a loop.\n");
}
