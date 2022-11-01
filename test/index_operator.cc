#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(IndexOperator, String)
{
    test_cellox_program("indexOperator/string.clx", "t\ne\ns\nt\n");
}

TEST(IndexOperator, WithBool)
{
    test_failing_cellox_program("indexOperator/with_bool.clx", "Operands must a number and a string\n[line 2] in script\n");
}