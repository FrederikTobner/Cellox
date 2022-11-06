#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Limits, LoopBodyTooLarge)
{
    test_failing_cellox_program("limits/loop_body_too_large.clx", "[line 2051] Error at '}': Loop body too large.\n");
}

TEST(Limits, ToManyConstants)
{
    test_failing_cellox_program("limits/too_many_constants.clx", "[line 34] Error at '256': Too many constants in one chunk.\n");
}

TEST(Limits, ToManyLocals)
{
    test_failing_cellox_program("limits/too_many_locals.clx", "[line 52] Error at 'v100': Too many local variables in function.\n");
}

TEST(Limits, ToManyUpvalues)
{
    test_failing_cellox_program("limits/too_many_upvalues.clx", "[line 102] Error at 'v100': Too many closure variables in function.\n");
}