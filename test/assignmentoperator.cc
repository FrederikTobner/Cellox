#include <gtest/gtest.h>
#include "TestUtils.hh"

extern "C"
{
#include "init.h"
}

TEST(AssgnmentOperator, Simple)
{
    test_program("assignmentOperators/simpleAssignment.clx", "5\ntest\ntrue\nnull\n");
}

TEST(AssgnmentOperator, MinusEqual)
{
    test_program("assignmentOperators/minusEqual.clx", "2\n");
}

TEST(AssgnmentOperator, PlusEqual)
{
    test_program("assignmentOperators/plusEqual.clx", "8\n");
}

TEST(AssgnmentOperator, StarEqual)
{
    test_program("assignmentOperators/starEqual.clx", "15\n");
}

TEST(AssgnmentOperator, DivideEqual)
{
    test_program("assignmentOperators/divideEqual.clx", "3\n");
}

TEST(AssgnmentOperator, ModuloEqual)
{
    test_program("assignmentOperators/moduloEqual.clx", "2\n");
}