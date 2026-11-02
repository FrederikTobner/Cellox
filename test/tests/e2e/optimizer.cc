#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Optimizer, ShortCircuitCondition) {
    test_cellox_program("optimizer/short_circuit_condition.clx", "taken\ntaken\nnot taken\nnot taken\n");
}

TEST(Optimizer, ConstantFoldingRespectsJumpTargets) {
    test_cellox_program("optimizer/constant_folding_jump_target.clx", "9\n3\n");
}

TEST(Optimizer, JumpOffsetsAreAdjustedAfterFolding) {
    test_cellox_program("optimizer/jump_offsets_after_folding.clx", "5\n18\n");
}
