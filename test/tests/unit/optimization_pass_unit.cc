#include "../fixtures/chunk_fixture.h"
#include <cstdlib>
#include <gtest/gtest.h>

extern "C" {
#include "backend/virtual_machine.h"
#include "byte-code/chunk.h"
#include "middle-end/optimization_pass.h"
}

namespace {

TEST_F(ChunkVMTest, ConstantFoldingFoldsBinaryAdditionBytecodeShape) {
    // Arrange
    uint8_t c0 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(2));
    uint8_t c1 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(3));

    emitConstant(c0);
    emitConstant(c1);
    emitSimple(OP_ADD);
    emitSimple(OP_RETURN);

    ASSERT_EQ(6u, chunk.byteCodeCount);

    // Act
    pass_result_t fold = pass_constant_folding(&chunk);

    // Assert
    EXPECT_TRUE(fold.modified);
    EXPECT_EQ(1u, fold.constants_folded);

    // Expected bytecode after fold: OP_CONSTANT c0, OP_RETURN
    ASSERT_EQ(3u, chunk.byteCodeCount);
    EXPECT_EQ(OP_CONSTANT, chunk.code[0]);
    EXPECT_EQ(c0, chunk.code[1]);
    EXPECT_EQ(OP_RETURN, chunk.code[2]);
    EXPECT_DOUBLE_EQ(5.0, AS_NUMBER(chunk.constants.values[c0]));
}

TEST_F(ChunkVMTest, ConstantFoldingDoesNotFoldDivisionByZero) {
    // Arrange
    uint8_t c0 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(10));
    uint8_t c1 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(0));
    emitConstant(c0);
    emitConstant(c1);
    emitSimple(OP_DIVIDE);
    emitSimple(OP_RETURN);

    // Act
    pass_result_t fold = pass_constant_folding(&chunk);

    // Assert
    EXPECT_FALSE(fold.modified);
    EXPECT_EQ(0u, fold.constants_folded);
    ASSERT_EQ(6u, chunk.byteCodeCount);
    EXPECT_EQ(OP_DIVIDE, chunk.code[4]);
}

TEST_F(ChunkVMTest, DeadCodeDetectionAndEliminationRemovesCodeAfterReturn) {
    // Arrange
    uint8_t c0 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(1));
    uint8_t c1 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(42));

    emitConstant(c0);      // reachable
    emitSimple(OP_RETURN); // terminal
    emitConstant(c1);      // dead
    emitSimple(OP_NEGATE); // dead

    // Act
    pass_result_t detect = pass_dead_code_detection(&chunk);

    // Assert
    EXPECT_GE(detect.instructions_removed, 1u);

    pass_result_t dce = pass_dead_code_elimination(&chunk);
    EXPECT_TRUE(dce.modified);

    // Expected remaining bytecode: OP_CONSTANT c0, OP_RETURN
    ASSERT_EQ(3u, chunk.byteCodeCount);
    EXPECT_EQ(OP_CONSTANT, chunk.code[0]);
    EXPECT_EQ(c0, chunk.code[1]);
    EXPECT_EQ(OP_RETURN, chunk.code[2]);
}

TEST_F(ChunkVMTest, PipelineRunKeepsCorrectFoldedBytecode) {
    // Arrange
    uint8_t c0 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(6));
    uint8_t c1 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(7));
    // Foldable expression.
    emitConstant(c0);
    emitConstant(c1);
    emitSimple(OP_MULTIPLY);
    emitSimple(OP_RETURN);

    optimization_module_init();

    // Act
    optimization_stats_t stats = optimization_pipeline_run_chunk(&chunk, "unit_pipeline");

    // Assert
    EXPECT_LE(stats.bytecode_size_after, stats.bytecode_size_before);

    ASSERT_EQ(3u, chunk.byteCodeCount);
    EXPECT_EQ(OP_CONSTANT, chunk.code[0]);
    EXPECT_EQ(c0, chunk.code[1]);
    EXPECT_EQ(OP_RETURN, chunk.code[2]);
    EXPECT_DOUBLE_EQ(42.0, AS_NUMBER(chunk.constants.values[c0]));

    free(stats.pass_results);
    optimization_module_cleanup();
}

TEST_F(ChunkVMTest, DeadCodeEliminationRetargetsForwardJump) {
    // Arrange
    uint8_t c0 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(7));

    // Layout before DCE:
    // 0: OP_JUMP 00 01   -> target 4
    // 3: OP_RETURN       (dead)
    // 4: OP_CONSTANT c0
    // 6: OP_RETURN
    emitSimple(OP_JUMP);
    emitSimple(0x00);
    emitSimple(0x01);
    emitSimple(OP_RETURN);
    emitConstant(c0);
    emitSimple(OP_RETURN);

    pass_result_t detect = pass_dead_code_detection(&chunk);
    EXPECT_GE(detect.instructions_removed, 1u);

    // Act
    pass_result_t dce = pass_dead_code_elimination(&chunk);

    // Assert
    EXPECT_TRUE(dce.modified);

    // Layout after DCE:
    // 0: OP_JUMP 00 00   -> target 3
    // 3: OP_CONSTANT c0
    // 5: OP_RETURN
    ASSERT_EQ(6u, chunk.byteCodeCount);
    EXPECT_EQ(OP_JUMP, chunk.code[0]);
    EXPECT_EQ(0x00, chunk.code[1]);
    EXPECT_EQ(0x00, chunk.code[2]);
    EXPECT_EQ(OP_CONSTANT, chunk.code[3]);
    EXPECT_EQ(c0, chunk.code[4]);
    EXPECT_EQ(OP_RETURN, chunk.code[5]);
}

TEST_F(ChunkVMTest, AlgebraicIdentityRemovesMultiplyByOneForNumericLhs) {
    // Arrange
    uint8_t c2 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(2));
    uint8_t c3 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(3));
    uint8_t c1 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(1));

    // CONST 2, CONST 3, SUBTRACT => numeric lhs, then * 1 identity.
    emitConstant(c2);
    emitConstant(c3);
    emitSimple(OP_SUBTRACT);
    emitConstant(c1);
    emitSimple(OP_MULTIPLY);
    emitSimple(OP_RETURN);

    ASSERT_EQ(9u, chunk.byteCodeCount);

    // Act
    pass_result_t id = pass_algebraic_identity(&chunk);

    // Assert
    EXPECT_TRUE(id.modified);

    ASSERT_EQ(6u, chunk.byteCodeCount);
    EXPECT_EQ(OP_CONSTANT, chunk.code[0]);
    EXPECT_EQ(c2, chunk.code[1]);
    EXPECT_EQ(OP_CONSTANT, chunk.code[2]);
    EXPECT_EQ(c3, chunk.code[3]);
    EXPECT_EQ(OP_SUBTRACT, chunk.code[4]);
    EXPECT_EQ(OP_RETURN, chunk.code[5]);
}

TEST_F(ChunkVMTest, BranchPredicationRewritesConstantTrueBranchToNoopJump) {
    // Arrange
    // TRUE, JUMP_IF_FALSE +5
    emitSimple(OP_TRUE);
    emitSimple(OP_JUMP_IF_FALSE);
    emitSimple(0x00);
    emitSimple(0x05);
    emitSimple(OP_RETURN);

    // Act
    pass_result_t pred = pass_branch_predication(&chunk);

    // Assert
    EXPECT_TRUE(pred.modified);
    EXPECT_EQ(1u, pred.branches_eliminated);

    ASSERT_EQ(5u, chunk.byteCodeCount);
    EXPECT_EQ(OP_TRUE, chunk.code[0]);
    EXPECT_EQ(OP_JUMP, chunk.code[1]);
    EXPECT_EQ(0x00, chunk.code[2]);
    EXPECT_EQ(0x00, chunk.code[3]);
}

} // namespace
