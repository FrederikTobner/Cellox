#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(ContinueStatement, WhileContinue) {
    test_cellox_program("continue/while_continue.clx", "1\n2\n4\n5\n");
}

TEST(ContinueStatement, ForContinue) {
    test_cellox_program("continue/for_continue.clx", "1\n2\n4\n5\n");
}

TEST(ContinueStatement, DoWhileContinue) {
    test_cellox_program("continue/do_while_continue.clx", "1\n2\n4\n5\n");
}

TEST(ContinueStatement, ContinueOutsideLoop) {
    test_failing_cellox_program("continue/continue_outside_loop.clx",
                                "[line 1] Error at 'continue': Can't use 'continue' outside of a loop.\n");
}
