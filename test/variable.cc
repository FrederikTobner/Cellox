#include "test_cellox.hh"

#include <gtest/gtest.h>

TEST(Variable, ClassAsIdentifier)
{
    test_failing_cellox_program("variable/class_as_identifier.clx", "[line 1] Error at 'class': Expect variable name.\n");
}

TEST(Variable, ColideWithParameter)
{
    test_failing_cellox_program("variable/collide_with_parameter.clx", "[line 2] Error at 'a': Already a variable with this name in this scope.\n");
}

TEST(Variable, Duplicate)
{
    test_failing_cellox_program("variable/duplicate.clx", "[line 3] Error at 'foo': Already a variable with this name in this scope.\n");
}

TEST(Variable, FunAsIdentifier)
{
    test_failing_cellox_program("variable/fun_as_identifier.clx", "[line 1] Error at 'fun': Expect variable name.\n");
}

TEST(Variable, NullAsIdentifier)
{
    test_failing_cellox_program("variable/null_as_identifier.clx", "[line 1] Error at 'null': Expect variable name.\n");
}

TEST(Variable, RedeclareGlobal)
{
    test_cellox_program("variable/redeclare_global.clx", "null\n");
}

TEST(Variable, ThisAsIdentifier)
{
    test_failing_cellox_program("variable/this_as_identifier.clx", "[line 1] Error at 'this': Expect variable name.\n");
}

TEST(Variable, TrueAsIdentifier)
{
    test_failing_cellox_program("variable/true_as_identifier.clx", "[line 1] Error at 'true': Expect variable name.\n");
}

TEST(Variable, Undefined)
{
    test_failing_cellox_program("variable/undefined.clx", "Undefined variable 'undefiened'.\n[line 1] in script\n");
}

TEST(Variable, Uninitialized)
{
    test_cellox_program("variable/uninitialized.clx", "null\n");
}

TEST(Variable, UnreachedUndefined)
{
    test_cellox_program("variable/unreached_undefiened.clx", "ok\n");
}