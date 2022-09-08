#include <gtest/gtest.h>

#include "test_cellox.hh"

extern "C"
{
#include "init.h"
}

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
    test_cellox_program("literalExpressions/strings.clx", "\na\nA\n123\n!\"$%&/()=?\n");
}
