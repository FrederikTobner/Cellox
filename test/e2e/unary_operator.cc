#include "test_cellox.hh"

#include <gtest/gtest.h>

TEST(UnaryOperator, Negate) {
    test_cellox_program("unary_operators/negate.clx", "-5\n10\n");
}

TEST(UnaryOperator, Not) {
    test_cellox_program("unary_operators/not.clx", "false\ntrue\n");
}

TEST(UnaryOperator, NegateNonNumber) {
    test_failing_cellox_program("unary_operators/negate_non_number.clx",
                                "Operand must be a number but is a boolean value.\n[line 1] in script\n");
}