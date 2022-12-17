#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(LogicalOperators, And)
{
    test_cellox_program("logical_operators/and.clx", "false\nfalse\ntrue\nfalse\nfalse\ntrue\n");
}

TEST(LogicalOperators, Or)
{
    test_cellox_program("logical_operators/or.clx", "false\ntrue\ntrue\nfalse\ntrue\ntrue\n");
}