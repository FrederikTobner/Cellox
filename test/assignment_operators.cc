#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(AssignmentOperator, Associativity) {
    test_cellox_program("assignment_operators/associativity.clx", "baz\nbaz\nbaz\n");
}

TEST(AssignmentOperator, Chained) {
    test_cellox_program("assignment_operators/chained.clx", "baz\nbaz\nbaz\n");
}

TEST(AssignmentOperator, DivideEqual) {
    test_cellox_program("assignment_operators/divide_equal.clx", "3\n-1.5\n");
}

TEST(AssignmentOperator, MinusEqual) {
    test_cellox_program("assignment_operators/minus_equal.clx", "2\n4\n");
}

TEST(AssignmentOperator, ModuloEqual) {
    test_cellox_program("assignment_operators/modulo_equal.clx", "2\n0\n");
}

TEST(AssignmentOperator, PlusEqual) {
    test_cellox_program("assignment_operators/plus_equal.clx", "8\n6\nhello world\n");
}

TEST(AssgnmentOperator, RaiseEqual) {
    test_cellox_program("assignment_operators/raise_equal.clx", "27\n3\n");
}

TEST(AssgnmentOperator, StarEqual) {
    test_cellox_program("assignment_operators/star_equal.clx", "15\n-22.5\n");
}

TEST(AssgnmentOperator, Simple) {
    test_cellox_program("assignment_operators/simple_assignment.clx", "5\ntest\ntrue\nnull\n");
}

TEST(AssgnmentOperator, ToBool) {
    test_failing_cellox_program("assignment_operators/to_bool.clx",
                                "[line 1] Error at '=': Invalid Token at the current position\n");
}

TEST(AssgnmentOperator, ToNull) {
    test_failing_cellox_program("assignment_operators/to_null.clx",
                                "[line 1] Error at '=': Invalid Token at the current position\n");
}

TEST(AssgnmentOperator, ToNumber) {
    test_failing_cellox_program("assignment_operators/to_number.clx",
                                "[line 1] Error at '=': Invalid Token at the current position\n");
}

TEST(AssgnmentOperator, ToString) {
    test_failing_cellox_program("assignment_operators/to_string.clx",
                                "[line 1] Error at '=': Invalid Token at the current position\n");
}