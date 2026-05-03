#include <gtest/gtest.h>

#include <cstdlib>

extern "C" {
#include "backend/virtual_machine.h"
#include "byte-code/chunk.h"
#include "middle-end/optimization_pass.h"
}

namespace {

static void EmitConstant(chunk_t * chunk, uint8_t constant_index, int32_t line = 1) {
    chunk_write(chunk, OP_CONSTANT, line);
    chunk_write(chunk, constant_index, line);
}

static void EmitSimple(chunk_t * chunk, uint8_t opcode, int32_t line = 1) {
    chunk_write(chunk, opcode, line);
}

TEST(OptimizationPassUnit, ConstantFoldingFoldsBinaryAdditionBytecodeShape) {
    virtual_machine_init();

    chunk_t chunk;
    chunk_init(&chunk);

    uint8_t c0 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(2));
    uint8_t c1 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(3));

    EmitConstant(&chunk, c0);
    EmitConstant(&chunk, c1);
    EmitSimple(&chunk, OP_ADD);
    EmitSimple(&chunk, OP_RETURN);

    ASSERT_EQ(6u, chunk.byteCodeCount);

    pass_result_t fold = pass_constant_folding(&chunk);

    EXPECT_TRUE(fold.modified);
    EXPECT_EQ(1u, fold.constants_folded);

    // Expected bytecode after fold: OP_CONSTANT c0, OP_RETURN
    ASSERT_EQ(3u, chunk.byteCodeCount);
    EXPECT_EQ(OP_CONSTANT, chunk.code[0]);
    EXPECT_EQ(c0, chunk.code[1]);
    EXPECT_EQ(OP_RETURN, chunk.code[2]);
    EXPECT_DOUBLE_EQ(5.0, AS_NUMBER(chunk.constants.values[c0]));

    chunk_free(&chunk);
    virtual_machine_free();
}

TEST(OptimizationPassUnit, ConstantFoldingDoesNotFoldDivisionByZero) {
    virtual_machine_init();

    chunk_t chunk;
    chunk_init(&chunk);

    uint8_t c0 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(10));
    uint8_t c1 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(0));
    EmitConstant(&chunk, c0);
    EmitConstant(&chunk, c1);
    EmitSimple(&chunk, OP_DIVIDE);
    EmitSimple(&chunk, OP_RETURN);

    pass_result_t fold = pass_constant_folding(&chunk);

    EXPECT_FALSE(fold.modified);
    EXPECT_EQ(0u, fold.constants_folded);
    ASSERT_EQ(6u, chunk.byteCodeCount);
    EXPECT_EQ(OP_DIVIDE, chunk.code[4]);

    chunk_free(&chunk);
    virtual_machine_free();
}

TEST(OptimizationPassUnit, DeadCodeDetectionAndEliminationRemovesCodeAfterReturn) {
    virtual_machine_init();

    chunk_t chunk;
    chunk_init(&chunk);

    uint8_t c0 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(1));
    uint8_t c1 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(42));

    EmitConstant(&chunk, c0);       // reachable
    EmitSimple(&chunk, OP_RETURN);  // terminal
    EmitConstant(&chunk, c1);       // dead
    EmitSimple(&chunk, OP_NEGATE);  // dead

    pass_result_t detect = pass_dead_code_detection(&chunk);
    EXPECT_GE(detect.instructions_removed, 1u);

    pass_result_t dce = pass_dead_code_elimination(&chunk);
    EXPECT_TRUE(dce.modified);

    // Expected remaining bytecode: OP_CONSTANT c0, OP_RETURN
    ASSERT_EQ(3u, chunk.byteCodeCount);
    EXPECT_EQ(OP_CONSTANT, chunk.code[0]);
    EXPECT_EQ(c0, chunk.code[1]);
    EXPECT_EQ(OP_RETURN, chunk.code[2]);

    chunk_free(&chunk);
    virtual_machine_free();
}

TEST(OptimizationPassUnit, PipelineRunKeepsCorrectFoldedBytecode) {
    virtual_machine_init();

    chunk_t chunk;
    chunk_init(&chunk);

    uint8_t c0 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(6));
    uint8_t c1 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(7));
    // Foldable expression.
    EmitConstant(&chunk, c0);
    EmitConstant(&chunk, c1);
    EmitSimple(&chunk, OP_MULTIPLY);
    EmitSimple(&chunk, OP_RETURN);

    optimization_module_init();
    optimization_stats_t stats = optimization_pipeline_run_chunk(&chunk, "unit_pipeline");

    EXPECT_LE(stats.bytecode_size_after, stats.bytecode_size_before);

    ASSERT_EQ(3u, chunk.byteCodeCount);
    EXPECT_EQ(OP_CONSTANT, chunk.code[0]);
    EXPECT_EQ(c0, chunk.code[1]);
    EXPECT_EQ(OP_RETURN, chunk.code[2]);
    EXPECT_DOUBLE_EQ(42.0, AS_NUMBER(chunk.constants.values[c0]));

    free(stats.pass_results);
    optimization_module_cleanup();
    chunk_free(&chunk);
    virtual_machine_free();
}

TEST(OptimizationPassUnit, DeadCodeEliminationRetargetsForwardJump) {
    virtual_machine_init();

    chunk_t chunk;
    chunk_init(&chunk);

    uint8_t c0 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(7));

    // Layout before DCE:
    // 0: OP_JUMP 00 01   -> target 4
    // 3: OP_RETURN       (dead)
    // 4: OP_CONSTANT c0
    // 6: OP_RETURN
    EmitSimple(&chunk, OP_JUMP);
    EmitSimple(&chunk, 0x00);
    EmitSimple(&chunk, 0x01);
    EmitSimple(&chunk, OP_RETURN);
    EmitConstant(&chunk, c0);
    EmitSimple(&chunk, OP_RETURN);

    pass_result_t detect = pass_dead_code_detection(&chunk);
    EXPECT_GE(detect.instructions_removed, 1u);

    pass_result_t dce = pass_dead_code_elimination(&chunk);
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

    chunk_free(&chunk);
    virtual_machine_free();
}

TEST(OptimizationPassUnit, AlgebraicIdentityRemovesMultiplyByOneForNumericLhs) {
    virtual_machine_init();

    chunk_t chunk;
    chunk_init(&chunk);

    uint8_t c2 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(2));
    uint8_t c3 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(3));
    uint8_t c1 = (uint8_t)chunk_add_constant(&chunk, NUMBER_VAL(1));

    // CONST 2, CONST 3, SUBTRACT => numeric lhs, then * 1 identity.
    EmitConstant(&chunk, c2);
    EmitConstant(&chunk, c3);
    EmitSimple(&chunk, OP_SUBTRACT);
    EmitConstant(&chunk, c1);
    EmitSimple(&chunk, OP_MULTIPLY);
    EmitSimple(&chunk, OP_RETURN);

    ASSERT_EQ(9u, chunk.byteCodeCount);

    pass_result_t id = pass_algebraic_identity(&chunk);
    EXPECT_TRUE(id.modified);

    ASSERT_EQ(6u, chunk.byteCodeCount);
    EXPECT_EQ(OP_CONSTANT, chunk.code[0]);
    EXPECT_EQ(c2, chunk.code[1]);
    EXPECT_EQ(OP_CONSTANT, chunk.code[2]);
    EXPECT_EQ(c3, chunk.code[3]);
    EXPECT_EQ(OP_SUBTRACT, chunk.code[4]);
    EXPECT_EQ(OP_RETURN, chunk.code[5]);

    chunk_free(&chunk);
    virtual_machine_free();
}

TEST(OptimizationPassUnit, BranchPredicationRewritesConstantTrueBranchToNoopJump) {
    virtual_machine_init();

    chunk_t chunk;
    chunk_init(&chunk);

    // TRUE, JUMP_IF_FALSE +5
    EmitSimple(&chunk, OP_TRUE);
    EmitSimple(&chunk, OP_JUMP_IF_FALSE);
    EmitSimple(&chunk, 0x00);
    EmitSimple(&chunk, 0x05);
    EmitSimple(&chunk, OP_RETURN);

    pass_result_t pred = pass_branch_predication(&chunk);
    EXPECT_TRUE(pred.modified);
    EXPECT_EQ(1u, pred.branches_eliminated);

    ASSERT_EQ(5u, chunk.byteCodeCount);
    EXPECT_EQ(OP_TRUE, chunk.code[0]);
    EXPECT_EQ(OP_JUMP, chunk.code[1]);
    EXPECT_EQ(0x00, chunk.code[2]);
    EXPECT_EQ(0x00, chunk.code[3]);

    chunk_free(&chunk);
    virtual_machine_free();
}

} // namespace
