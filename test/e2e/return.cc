#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Return, AtTopLevel) {
    test_failing_cellox_program("return/at_top_level.clx",
                                "[line 1] Error at 'return': You can't use return from top-level code.\n");
}