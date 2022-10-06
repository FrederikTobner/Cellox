#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(AssgnmentOperator, Simple)
{
    test_cellox_program("assignmentOperators/simpleAssignment.clx", "5\ntest\ntrue\nnull\n", false);
}

TEST(AssgnmentOperator, MinusEqual)
{
    test_cellox_program("assignmentOperators/minusEqual.clx", "2\n", false);
}

TEST(AssgnmentOperator, PlusEqual)
{
    test_cellox_program("assignmentOperators/plusEqual.clx", "8\nhello world\n", false);
}

TEST(AssgnmentOperator, StarEqual)
{
    test_cellox_program("assignmentOperators/starEqual.clx", "15\n", false);
}

TEST(AssgnmentOperator, DivideEqual)
{
    test_cellox_program("assignmentOperators/divideEqual.clx", "3\n", false);
}

TEST(AssgnmentOperator, ModuloEqual)
{
    test_cellox_program("assignmentOperators/moduloEqual.clx", "2\n", false);
}

TEST(AssgnmentOperator, RaiseEqual)
{
    test_cellox_program("assignmentOperators/raiseEqual.clx", "27\n", false);
}