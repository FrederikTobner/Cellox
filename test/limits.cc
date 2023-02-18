#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Limits, FunTooManyArguments) {
    test_failing_cellox_program("limits/fun_too_many_arguments.clx", "[line 15] Error at '256': Can't have more than 255 arguments in a function call.\n");
}

TEST(Limits, LoopBodyTooLarge) {
    test_failing_cellox_program("limits/loop_body_too_large.clx", "[line 2051] Error at '}': Loop body too large.\n");
}

TEST(Limits, MethodTooManyArguments) {
    test_failing_cellox_program("limits/method_too_many_arguments.clx", "[line 259] Error at 'a': Can't have more than 255 arguments in a function call.\n");
}

TEST(Limits, MethodTooManyParameters) {
    test_failing_cellox_program("limits/method_too_many_parameters.clx", "[line 259] Error at 'param256': Can't have more than 255 parameters.\n");
}

TEST(Limits, TooLongArrayLiteral) {
    test_failing_cellox_program("limits/too_long_array_literal.clx",
                                "[line 15] Error at '256': Can't have more than 255 arguments in a array literal expression.\n");
}

TEST(Limits, TooManyConstants) {
    test_failing_cellox_program("limits/too_many_constants.clx", "[line 34] Error at '256': Too many constants in one chunk.\n");
}

TEST(Limits, TooManyLocals) {
    test_failing_cellox_program("limits/too_many_locals.clx", "[line 52] Error at 'v100': Too many local variables in function.\n");
}

TEST(Limits, TooManyUpvalues) {
    test_failing_cellox_program("limits/too_many_upvalues.clx", "[line 102] Error at 'v100': Too many closure variables in function.\n");
}
