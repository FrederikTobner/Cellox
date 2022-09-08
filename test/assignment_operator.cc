#include <gtest/gtest.h>
#include "test_cellox.hh"

extern "C"
{
#include "init.h"
}

TEST(AssgnmentOperator, Simple)
{
    test_cellox_program("assignmentOperators/simpleAssignment.clx", "5\ntest\ntrue\nnull\n");
}

TEST(AssgnmentOperator, MinusEqual)
{
    test_cellox_program("assignmentOperators/minusEqual.clx", "2\n");
}

TEST(AssgnmentOperator, PlusEqual)
{
    test_cellox_program("assignmentOperators/plusEqual.clx", "8\nhello world\n");
}

TEST(AssgnmentOperator, StarEqual)
{
    test_cellox_program("assignmentOperators/starEqual.clx", "15\n");
}

TEST(AssgnmentOperator, DivideEqual)
{
    test_cellox_program("assignmentOperators/divideEqual.clx", "3\n");
}

TEST(AssgnmentOperator, ModuloEqual)
{
    test_cellox_program("assignmentOperators/moduloEqual.clx", "2\n");
}

TEST(AssgnmentOperator, RaiseEqual)
{
    test_cellox_program("assignmentOperators/raiseEqual.clx", "27\n");
}