#include <gtest/gtest.h>
#include "test_cellox.hh"

TEST(LiteralExpressions, Booleans)
{
    test_cellox_program("literalExpressions/booleans.clx", "true\nfalse\n");
}

TEST(LiteralExpressions, Null)
{
    test_cellox_program("literalExpressions/null.clx", "null\n");
}

TEST(LiteralExpressions, Numbers)
{
    test_cellox_program("literalExpressions/numbers.clx", "123\n-10\n123.456\n-0.001\n");
}

TEST(LiteralExpressions, Strings)
{
    #ifdef _WIN32
        test_cellox_program("literalExpressions/strings.clx", "\na\nA\n123\n!\"$%&/()=?\n1\r\n2\r\n3\n");
    #endif
    #ifdef linux
        test_cellox_program("literalExpressions/strings.clx", "\na\nA\n123\n!\"$%&/()=?\n1\n2\n3\n");
    #endif 
}
