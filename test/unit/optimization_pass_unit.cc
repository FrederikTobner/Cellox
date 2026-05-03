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

} // namespace
