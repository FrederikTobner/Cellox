#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Limits, ToManyConstants)
{
    test_failing_cellox_program("limits/to_many_constants.clx", "[line 34] Error at '256': Too many constants in one chunk.\n");
}

TEST(Limits, ToManyLocals)
{
    test_failing_cellox_program("limits/to_many_locals.clx", "[line 52] Error at 'v100': Too many local variables in function.\n");
}