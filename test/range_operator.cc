#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(RangeOperator, WithBool)
{
    test_failing_cellox_program("range_operator/with_boolean.clx", "A range can only be created with a number as first argument but was created with a boolean object\n[line 2] in script\n");
}

TEST(RangeOperator, WithClass)
{
    test_failing_cellox_program("range_operator/with_class.clx", "A range can only be created with a number as first argument but was created with a class object\n[line 3] in script\n");
}

TEST(RangeOperator, WithFunction)
{
    test_failing_cellox_program("range_operator/with_function.clx", "A range can only be created with a number as first argument but was created with a closure object\n[line 3] in script\n");
}

TEST(RangeOperator, WithNull)
{
    test_failing_cellox_program("range_operator/with_null.clx", "A range can only be created with a number as first argument but was created with a undefiened object\n[line 2] in script\n");
}

TEST(RangeOperator, WithString)
{
    test_failing_cellox_program("range_operator/with_string.clx", "A range can only be created with a number as first argument but was created with a string object\n[line 2] in script\n");
}