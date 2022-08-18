#include <gtest/gtest.h>
#include "TestUtils.hh"

extern "C"
{
#include "init.h"
}

TEST(LiteralExpressions, Booleans)
{
    test_program("literalExpressions/booleans.clx", "true\nfalse\n");
}

TEST(LiteralExpressions, Null)
{
    test_program("literalExpressions/null.clx", "null\n");
}

TEST(LiteralExpressions, Numbers)
{
    test_program("literalExpressions/numbers.clx", "123\n-10\n123.456\n-0.001\n");
}

TEST(LiteralExpressions, Strings)
{
    test_program("literalExpressions/strings.clx", "\na\nA\n123\n!\"$%&/()=?\n");
}
