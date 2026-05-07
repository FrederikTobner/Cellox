/**
 * @file test_optimization_passes.c
 * @brief Comprehensive unit tests for optimization passes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "byte-code/chunk.h"
#include "language-models/value.h"
#include "middle-end/optimization_pass.h"

static int g_test_count = 0;
static int g_test_passed = 0;
static int g_test_failed = 0;

#define ASSERT_EQ(actual, expected, format) \
    do { \
        g_test_count++; \
        if ((actual) != (expected)) { \
            g_test_failed++; \
            printf("  FAIL at %s:%d: ", __FILE__, __LINE__); \
            printf("Expected " format ", got " format "\n", (expected), (actual)); \
        } else { \
            g_test_passed++; \
        } \
    } while (0)

#define ASSERT_TRUE(cond, msg) \
    do { \
        g_test_count++; \
        if (!(cond)) { \
            g_test_failed++; \
            printf("  FAIL at %s:%d: %s\n", __FILE__, __LINE__, (msg)); \
        } else { \
            g_test_passed++; \
        } \
    } while (0)

#define SECTION(name) printf("\n=== %s ===\n", (name))
#define TEST_PASS() printf("✓ PASS\n")

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Emit a constant bytecode instruction
 */
static void emit_constant(chunk_t* chunk, value_t value) {
    int32_t index = chunk_add_constant(chunk, value);
    chunk_write(chunk, OP_CONSTANT, 0);
    chunk_write(chunk, (uint8_t)index, 0);
}

/**
 * @brief Emit a binary operation bytecode instruction
 */
static void emit_op(chunk_t* chunk, uint8_t opcode) {
    chunk_write(chunk, opcode, 0);
}

/**
 * @brief Emit a return instruction
 */
static void emit_return(chunk_t* chunk) {
    emit_op(chunk, OP_NULL);
    emit_op(chunk, OP_RETURN);
}

/**
 * @brief Print bytecode for debugging
 */
static void print_chunk(chunk_t* chunk, const char* name) {
    printf("  Chunk: %s\n", name);
    printf("  Bytecode count: %u\n", chunk->byteCodeCount);
    printf("  Constants count: %u\n", chunk->constants.count);
    
    printf("  Bytecode: ");
    for (uint32_t i = 0; i < chunk->byteCodeCount; i++) {
        printf("%02X ", chunk->code[i]);
    }
    printf("\n");
    
    printf("  Constants: ");
    for (size_t i = 0; i < chunk->constants.count; i++) {
        if (IS_NUMBER(chunk->constants.values[i])) {
            printf("%.0f ", AS_NUMBER(chunk->constants.values[i]));
        } else if (IS_BOOL(chunk->constants.values[i])) {
            printf("%s ", AS_BOOL(chunk->constants.values[i]) ? "true" : "false");
        } else {
            printf("? ");
        }
    }
    printf("\n");
}

// ============================================================================
// Test Cases
// ============================================================================

/**
 * @test Constant folding: 5 + 3 → 8
 */
static void test_constant_folding_addition(void) {
    chunk_t chunk;
    chunk_init(&chunk);
    
    // Generate: CONST 5, CONST 3, ADD
    emit_constant(&chunk, NUMBER_VAL(5.0));
    emit_constant(&chunk, NUMBER_VAL(3.0));
    emit_op(&chunk, OP_ADD);
    emit_return(&chunk);
    
    uint32_t bytecode_before = chunk.byteCodeCount;
    
    print_chunk(&chunk, "before folding");
    
    // Run constant folding
    pass_result_t result = pass_constant_folding(&chunk);
    
    print_chunk(&chunk, "after folding");
    
    printf("  Folded: %s\n", result.modified ? "yes" : "no");
    printf("  Constants folded: %zu\n", result.constants_folded);
    printf("  Bytecode removed: %u → %u\n", bytecode_before, chunk.byteCodeCount);
    
    // After folding: CONST 8, ADD should be removed, leaving CONST 8 + return
    ASSERT_TRUE(result.modified, "Constant folding should modify");
    ASSERT_TRUE(result.constants_folded > 0, "Should fold at least one constant");
    ASSERT_TRUE(chunk.byteCodeCount < bytecode_before, "Bytecode should be smaller");
    
    chunk_free(&chunk);
    TEST_PASS();
}

/**
 * @test Constant folding: multiple operations (5 + 3) * 2
 */
static void test_constant_folding_recursive(void) {
    chunk_t chunk;
    chunk_init(&chunk);
    
    // Generate: CONST 5, CONST 3, ADD, CONST 2, MULTIPLY
    emit_constant(&chunk, NUMBER_VAL(5.0));
    emit_constant(&chunk, NUMBER_VAL(3.0));
    emit_op(&chunk, OP_ADD);
    emit_constant(&chunk, NUMBER_VAL(2.0));
    emit_op(&chunk, OP_MULTIPLY);
    emit_return(&chunk);
    
    print_chunk(&chunk, "before recursive folding");
    
    // Run constant folding
    pass_result_t result = pass_constant_folding(&chunk);
    
    print_chunk(&chunk, "after recursive folding");
    
    printf("  Constants folded: %zu\n", result.constants_folded);
    
    // Should fold both operations
    ASSERT_TRUE(result.modified, "Should fold");
    ASSERT_TRUE(result.constants_folded >= 2, "Should fold at least twice");
    
    chunk_free(&chunk);
    TEST_PASS();
}

/**
 * @test Constant folding: division by zero (must not fold)
 */
static void test_constant_folding_division_by_zero(void) {
    chunk_t chunk;
    chunk_init(&chunk);
    
    // Generate: CONST 10, CONST 0, DIVIDE
    emit_constant(&chunk, NUMBER_VAL(10.0));
    emit_constant(&chunk, NUMBER_VAL(0.0));
    emit_op(&chunk, OP_DIVIDE);
    emit_return(&chunk);
    
    uint32_t bytecode_before = chunk.byteCodeCount;
    
    print_chunk(&chunk, "before div-by-zero check");
    
    // Run constant folding
    pass_result_t result = pass_constant_folding(&chunk);
    
    print_chunk(&chunk, "after div-by-zero check");
    
    // Division by zero should NOT be folded
    ASSERT_TRUE(chunk.byteCodeCount == bytecode_before, "Bytecode should NOT be modified");
    ASSERT_TRUE(result.constants_folded == 0, "Should NOT fold division by zero");
    
    chunk_free(&chunk);
    TEST_PASS();
}

/**
 * @test Dead code detection: code after return
 */
static void test_dead_code_detection(void) {
    chunk_t chunk;
    chunk_init(&chunk);
    
    // Generate: CONST 1, RETURN, CONST 2 (unreachable)
    emit_constant(&chunk, NUMBER_VAL(1.0));
    emit_op(&chunk, OP_RETURN);
    emit_constant(&chunk, NUMBER_VAL(2.0));  // This is unreachable
    
    print_chunk(&chunk, "before dead code detection");
    
    // Run dead code detection
    pass_result_t result = pass_dead_code_detection(&chunk);
    
    printf("  Unreachable instructions: %zu\n", result.instructions_removed);
    
    ASSERT_TRUE(result.instructions_removed > 0, "Should detect dead code after return");
    
    chunk_free(&chunk);
    TEST_PASS();
}

/**
 * @test Dead code elimination: remove code after return
 */
static void test_dead_code_elimination(void) {
    chunk_t chunk;
    chunk_init(&chunk);
    
    // Generate: CONST 1, RETURN, CONST 2, OP
    emit_constant(&chunk, NUMBER_VAL(1.0));
    emit_op(&chunk, OP_RETURN);
    emit_constant(&chunk, NUMBER_VAL(2.0));
    emit_op(&chunk, OP_POP);
    
    uint32_t bytecode_before = chunk.byteCodeCount;
    
    print_chunk(&chunk, "before dead code elimination");
    
    // Run detection first
    pass_dead_code_detection(&chunk);
    
    // Run elimination
    pass_result_t result = pass_dead_code_elimination(&chunk);
    
    print_chunk(&chunk, "after dead code elimination");
    
    printf("  Instructions removed: %zu\n", result.instructions_removed);
    printf("  Bytecode: %u → %u\n", bytecode_before, chunk.byteCodeCount);
    
    ASSERT_TRUE(result.modified, "Dead code elimination should modify");
    ASSERT_TRUE(chunk.byteCodeCount < bytecode_before, "Bytecode should be smaller");
    
    chunk_free(&chunk);
    TEST_PASS();
}

/**
 * @test Pipeline: multiple passes run in sequence  
 */
static void test_optimization_pipeline(void) {
    chunk_t chunk;
    chunk_init(&chunk);
    
    // Generate code with both dead code and constant folding opportunities
    emit_constant(&chunk, NUMBER_VAL(5.0));
    emit_constant(&chunk, NUMBER_VAL(3.0));
    emit_op(&chunk, OP_ADD);
    emit_op(&chunk, OP_RETURN);
    emit_constant(&chunk, NUMBER_VAL(999.0));  // Dead code
    
    uint32_t bytecode_before = chunk.byteCodeCount;
    
    print_chunk(&chunk, "before pipeline");
    
    // Initialize and run the full pipeline
    optimization_module_init();
    optimization_stats_t stats = optimization_pipeline_run_chunk(&chunk, "test_pipeline");
    
    print_chunk(&chunk, "after pipeline");
    
    printf("  Bytecode: %zu → %zu\n", stats.bytecode_size_before, stats.bytecode_size_after);
    printf("  Constants: %zu → %zu\n", stats.constants_before, stats.constants_after);
    printf("  Time: %lu ns\n", stats.time_ns);
    
    ASSERT_TRUE(chunk.byteCodeCount < bytecode_before, "Pipeline should reduce bytecode");
    
    chunk_free(&chunk);
    free(stats.pass_results);
    TEST_PASS();
}

static void test_constant_pool_dedup(void) {
    chunk_t chunk;
    chunk_init(&chunk);

    // Emit: CONST 42, CONST 42, ADD, RETURN
    emit_constant(&chunk, NUMBER_VAL(42.0));
    emit_constant(&chunk, NUMBER_VAL(42.0)); // duplicate
    emit_op(&chunk, OP_ADD);
    emit_return(&chunk);

    size_t constants_before = chunk.constants.count;
    print_chunk(&chunk, "before dedup");

    // Run deduplication
    pass_result_t result = pass_constant_pool_dedup(&chunk);

    print_chunk(&chunk, "after dedup");

    ASSERT_TRUE(result.modified, "Dedup should modify chunk");
    ASSERT_TRUE(chunk.constants.count < constants_before, "Should reduce constant count");
    // All constant references should now point to the same index (0)
    ASSERT_EQ(chunk.code[1], 0, "%d");
    ASSERT_EQ(chunk.code[3], 0, "%d");

    chunk_free(&chunk);
    TEST_PASS();
}

int main(void) {
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║  Cellox Optimization Pass Test Suite                  ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    SECTION("Constant Folding Tests");
    test_constant_folding_addition();
    test_constant_folding_recursive();
    test_constant_folding_division_by_zero();
    
    SECTION("Dead Code Detection Tests");
    test_dead_code_detection();
    
    SECTION("Dead Code Elimination Tests");
    test_dead_code_elimination();
    
    SECTION("Constant Pool Deduplication Tests");
    test_constant_pool_dedup();

    SECTION("Pipeline Integration Tests");
    test_optimization_pipeline();
    
    optimization_module_cleanup();
    
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║  Test Results: %d passed, %d failed (total: %d)          \n",
        g_test_passed, g_test_failed, g_test_count);
    printf("╚════════════════════════════════════════════════════════╝\n\n");
    
    return g_test_failed > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
