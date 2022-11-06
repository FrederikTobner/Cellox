#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(IndexOperator, ChangeStringByIndex)
{
    test_cellox_program("indexOperator/change_string_by_index.clx", "celiop\ncellop\ncellox\n");
}

TEST(IndexOperator, OutOfBounds)
{
    test_failing_cellox_program("indexOperator/out_of_bounds.clx", "accessed string out of bounds\n[line 2] in script\n");
}

TEST(IndexOperator, String)
{
    test_cellox_program("indexOperator/string.clx", "t\ne\ns\nt\n");
}

TEST(IndexOperator, WithBool)
{
    test_failing_cellox_program("indexOperator/with_bool.clx", "Operands must a number and a string but was called with a boolean value and a string value\n[line 2] in script\n");
}

TEST(IndexOperator, WithClass)
{
    test_failing_cellox_program("indexOperator/with_class.clx", "Operands must a number and a string but was called with a class value and a string value\n[line 3] in script\n");
}

TEST(IndexOperator, WithFunction)
{
    test_failing_cellox_program("indexOperator/with_function.clx", "Operands must a number and a string but was called with a closure value and a string value\n[line 3] in script\n");
}

TEST(IndexOperator, WithNull)
{
    test_failing_cellox_program("indexOperator/with_null.clx", "Operands must a number and a string but was called with a undefiened value and a string value\n[line 2] in script\n");
}

TEST(IndexOperator, WithString)
{
    test_failing_cellox_program("indexOperator/with_string.clx", "Operands must a number and a string but was called with a string value and a string value\n[line 2] in script\n");
}