#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(AssgnmentOperator, Associativity)
{
    test_cellox_program("assignmentOperators/associativity.clx", "baz\nbaz\nbaz\n");
}

TEST(AssgnmentOperator, Chained)
{
    test_cellox_program("assignmentOperators/chained.clx", "baz\nbaz\nbaz\n");
}

TEST(AssgnmentOperator, DivideEqual)
{
    test_cellox_program("assignmentOperators/divide_equal.clx", "3\n-1.5\n");
}

TEST(AssgnmentOperator, MinusEqual)
{
    test_cellox_program("assignmentOperators/minus_equal.clx", "2\n4\n");
}

TEST(AssgnmentOperator, ModuloEqual)
{
    test_cellox_program("assignmentOperators/modulo_equal.clx", "2\n0\n");
}

TEST(AssgnmentOperator, PlusEqual)
{
    test_cellox_program("assignmentOperators/plus_equal.clx", "8\n6\nhello world\n");
}

TEST(AssgnmentOperator, RaiseEqual)
{
    test_cellox_program("assignmentOperators/raise_equal.clx", "27\n3\n");
}

TEST(AssgnmentOperator, StarEqual)
{
    test_cellox_program("assignmentOperators/star_equal.clx", "15\n-22.5\n");
}

TEST(AssgnmentOperator, Simple)
{
    test_cellox_program("assignmentOperators/simple_assignment.clx", "5\ntest\ntrue\nnull\n");
}

TEST(AssgnmentOperator, ToBool)
{
    test_failing_cellox_program("assignmentOperators/to_bool.clx", "[line 1] Error at '=': Invalid Token at the current position\n");
}

TEST(AssgnmentOperator, ToNull)
{
    test_failing_cellox_program("assignmentOperators/to_null.clx", "[line 1] Error at '=': Invalid Token at the current position\n");
}

TEST(AssgnmentOperator, ToNumber)
{
    test_failing_cellox_program("assignmentOperators/to_number.clx", "[line 1] Error at '=': Invalid Token at the current position\n");
}

TEST(AssgnmentOperator, ToString)
{
    test_failing_cellox_program("assignmentOperators/to_string.clx", "[line 1] Error at '=': Invalid Token at the current position\n");
}