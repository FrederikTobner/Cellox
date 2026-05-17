#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(LogicalOperators, And) {
    test_cellox_program("logical_operators/and.clx", "false\nfalse\ntrue\nfalse\nfalse\ntrue\n");
}

TEST(LogicalOperators, Or) {
    test_cellox_program("logical_operators/or.clx", "false\ntrue\ntrue\nfalse\ntrue\ntrue\n");
}

TEST(LogicalOperators, ShortCircuitAnd) {
    test_cellox_program("logical_operators/short_circuit_and.clx", "false\n");
}

TEST(LogicalOperators, ShortCircuitOr) {
    test_cellox_program("logical_operators/short_circuit_or.clx", "true\n");
}