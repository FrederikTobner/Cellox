#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(AssgnmentOperator, Simple)
{
    test_cellox_program("assignmentOperators/simple_assignment.clx", "5\ntest\ntrue\nnull\n", false);
}

TEST(AssgnmentOperator, MinusEqual)
{
    test_cellox_program("assignmentOperators/minus_equal.clx", "2\n", false);
}

TEST(AssgnmentOperator, PlusEqual)
{
    test_cellox_program("assignmentOperators/plus_equal.clx", "8\nhello world\n", false);
}

TEST(AssgnmentOperator, StarEqual)
{
    test_cellox_program("assignmentOperators/star_equal.clx", "15\n", false);
}

TEST(AssgnmentOperator, DivideEqual)
{
    test_cellox_program("assignmentOperators/divide_equal.clx", "3\n", false);
}

TEST(AssgnmentOperator, ModuloEqual)
{
    test_cellox_program("assignmentOperators/modulo_equal.clx", "2\n", false);
}

TEST(AssgnmentOperator, RaiseEqual)
{
    test_cellox_program("assignmentOperators/raise_equal.clx", "27\n", false);
}