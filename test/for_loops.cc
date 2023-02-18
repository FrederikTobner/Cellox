#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(ForLoops, NoCounterIncrementExpression) {
    test_cellox_program("forLoops/no_counter_increment_expression.clx", "1\n2\n3\n4\n");
}

TEST(ForLoops, NoVariable) {
    test_cellox_program("forLoops/no_variable.clx", "0\n1\n2\n");
}

TEST(ForLoops, NormalFor) {
    test_cellox_program("forLoops/normal_for.clx", "1\n2\n3\n4\n5\n");
}