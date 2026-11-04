#include <gtest/gtest.h>

#include "test_cellox.hpp"

TEST(Optimizer, ShortCircuitCondition) {
    test_cellox_program("optimizer/short_circuit_condition.clx", "taken\ntaken\nnot taken\nnot taken\n");
}

TEST(Optimizer, ConstantFoldingRespectsJumpTargets) {
    test_cellox_program("optimizer/constant_folding_jump_target.clx", "9\n3\n");
}

TEST(Optimizer, JumpOffsetsAreAdjustedAfterFolding) {
    test_cellox_program("optimizer/jump_offsets_after_folding.clx", "5\n18\n");
}

TEST(Optimizer, ConstantFolding) {
    test_cellox_program("optimizer/constant_folding.clx", "11");
}

TEST(Optimizer, AlgebraicIdentityAddZero) {
    test_cellox_program("optimizer/algebraic_identity_add_zero.clx", "42");
}

TEST(Optimizer, AlgebraicIdentitySubtractZero) {
    test_cellox_program("optimizer/algebraic_identity_subtract_zero.clx", "42");
}

TEST(Optimizer, AlgebraicIdentityMultiplyOne) {
    test_cellox_program("optimizer/algebraic_identity_multiply_one.clx", "7");
}

TEST(Optimizer, AlgebraicIdentityDivideOne) {
    test_cellox_program("optimizer/algebraic_identity_divide_one.clx", "84");
}

TEST(Optimizer, BranchPredicationTrueCondition) {
    test_cellox_program("optimizer/branch_predication_true_condition.clx", "1");
}

TEST(Optimizer, BranchPredicationFalseCondition) {
    test_cellox_program("optimizer/branch_predication_false_condition.clx", "2");
}

TEST(Optimizer, DeadCodeEliminationAfterReturn) {
    test_cellox_program("optimizer/dead_code_elimination_after_return.clx", "11");
}

TEST(Optimizer, DeadCodeEliminationAfterUnconditionalJump) {
    test_cellox_program("optimizer/dead_code_elimination_after_unconditional_jump.clx", "1030");
}
