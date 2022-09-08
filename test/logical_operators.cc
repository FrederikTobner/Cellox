#include <gtest/gtest.h>
#include "test_cellox.hh"

extern "C"
{
#include "init.h"
}

TEST(LogicalOperators, And)
{
    test_cellox_program("logicalOperators/and.clx", "false\nfalse\ntrue\nfalse\nfalse\ntrue\n");
}

TEST(LogicalOperators, Or)
{
    test_cellox_program("logicalOperators/or.clx", "false\ntrue\ntrue\nfalse\ntrue\ntrue\n");
}