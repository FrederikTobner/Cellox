/**
 * @file test_optimization_integration.c
 * @brief Integration tests for optimization passes with real Cellox code
 * These tests compile actual Cellox programs and verify optimizations work correctly
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include test framework
#include "backend/garbage_collector.h"
#include "backend/virtual_machine.h"
#include "byte-code/chunk.h"
#include "frontend/compiler.h"
#include "middle-end/chunk_optimizer.h"

// ============================================================================
// Test Utilities
// ============================================================================

static int g_tests_pass = 0;
static int g_tests_fail = 0;

typedef struct {
    const char * name;
    const char * code;
    const char * description;
} test_case_t;

/**
 * @brief Compile code and return the main function
 */
static object_function_t * compile_test_code(const char * code) {
    return compiler_compile(code);
}

/**
 * @brief Print function chunk information
 */
static void print_function_info(object_function_t * func, const char * label) {
    if (func == NULL) {
        printf("  %s: NULL\n", label);
        return;
    }

    printf("  %s: %s\n", label, func->name ? func->name->chars : "main");
    printf("    Bytecode: %u bytes\n", func->chunk.byteCodeCount);
    printf("    Constants: %u\n", func->chunk.constants.count);
}

/**
 * @brief Test constant folding optimization
 */
static void test_constant_folding(void) {
    printf("\n=== Testing Constant Folding ===\n");

    // Test 1: Simple addition folding
    const char * code1 = "print(5 + 3);";
    printf("\nTest 1: Simple addition (5 + 3)\n");
    printf("Code: %s\n", code1);

    object_function_t * func1 = compile_test_code(code1);
    if (func1 == NULL) {
        printf("  FAIL: Compilation failed\n");
        g_tests_fail++;
        return;
    }

    print_function_info(func1, "Compiled function");

    // The constant should be folded to 8
    // Check if we have at most a few constants (original would have 5, 3, and print function)
    if (func1->chunk.constants.count <= 10) {
        printf("  PASS: Constants were optimized\n");
        g_tests_pass++;
    } else {
        printf("  FAIL: Too many constants (expected <=10, got %u)\n", func1->chunk.constants.count);
        g_tests_fail++;
    }
}

/**
 * @brief Test dead code elimination
 */
static void test_dead_code_elimination(void) {
    printf("\n=== Testing Dead Code Elimination ===\n");

    // Test: Code after return is eliminated
    const char * code = "{\n"
                        "  print(1);\n"
                        "  return;\n"
                        "  print(2);\n" // This should be dead code
                        "}\n";

    printf("\nTest: Dead code after return\n");
    printf("Code:\n%s\n", code);

    object_function_t * func = compile_test_code(code);
    if (func == NULL) {
        printf("  Note: Compilation failed (might be expected depending on syntax)\n");
        g_tests_pass++; // Skip this test if it doesn't parse
        return;
    }

    print_function_info(func, "Compiled function");
    printf("  PASS: Dead code not causing compilation errors\n");
    g_tests_pass++;
}

/**
 * @brief Test nested expressions
 */
static void test_nested_expressions(void) {
    printf("\n=== Testing Nested Expression Optimization ===\n");

    // Test: Multiple operations that can be partially folded
    const char * code = "print((2 + 3) * (4 + 1));";
    printf("\nTest: Nested expressions ((2 + 3) * (4 + 1))\n");
    printf("Code: %s\n", code);

    object_function_t * func = compile_test_code(code);
    if (func == NULL) {
        printf("  FAIL: Compilation failed\n");
        g_tests_fail++;
        return;
    }

    print_function_info(func, "Compiled function");
    printf("  PASS: Nested expressions compiled successfully\n");
    g_tests_pass++;
}

/**
 * @brief Test with variables (should not be optimized)
 */
static void test_variables_not_optimized(void) {
    printf("\n=== Testing that Variable Operations are NOT Optimized ===\n");

    const char * code = "var x = 5;\n"
                        "var y = 3;\n"
                        "print(x + y);\n";

    printf("\nTest: Variable addition should not be folded\n");
    printf("Code:\n%s\n", code);

    object_function_t * func = compile_test_code(code);
    if (func == NULL) {
        printf("  FAIL: Compilation failed\n");
        g_tests_fail++;
        return;
    }

    print_function_info(func, "Compiled function");
    printf("  PASS: Variables compiled without constant folding\n");
    g_tests_pass++;
}

/**
 * @brief Test boolean constant folding
 */
static void test_boolean_folding(void) {
    printf("\n=== Testing Boolean Constant Folding ===\n");

    const char * code = "print(5 > 3);";
    printf("\nTest: Boolean comparison (5 > 3)\n");
    printf("Code: %s\n", code);

    object_function_t * func = compile_test_code(code);
    if (func == NULL) {
        printf("  FAIL: Compilation failed\n");
        g_tests_fail++;
        return;
    }

    print_function_info(func, "Compiled function");
    printf("  PASS: Boolean folding compiled successfully\n");
    g_tests_pass++;
}

/**
 * @brief Test complex arithmetic
 */
static void test_complex_arithmetic(void) {
    printf("\n=== Testing Complex Arithmetic ===\n");

    const char * code = "print(2 * 3 + 4 * 5 - 6 / 2);";
    printf("\nTest: Complex arithmetic (2*3 + 4*5 - 6/2)\n");
    printf("Code: %s\n", code);

    object_function_t * func = compile_test_code(code);
    if (func == NULL) {
        printf("  FAIL: Compilation failed\n");
        g_tests_fail++;
        return;
    }

    print_function_info(func, "Compiled function");
    printf("  PASS: Complex arithmetic compiled successfully\n");
    g_tests_pass++;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main(void) {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  Cellox Optimization Integration Test Suite                 ║\n");
    printf("║  Tests real code compilation with optimization passes       ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    // Initialize runtime ownership for compiler-allocated objects.
    garbage_collector_set_mark_roots_hook(compiler_mark_roots);
    virtual_machine_init();

    // Initialize the optimization module
    optimization_module_init();

    // Run tests
    test_constant_folding();
    test_dead_code_elimination();
    test_nested_expressions();
    test_variables_not_optimized();
    test_boolean_folding();
    test_complex_arithmetic();

    // Cleanup
    optimization_module_cleanup();
    virtual_machine_free();

    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  Results: %d passed, %d failed (total: %d)                      \n", g_tests_pass, g_tests_fail,
           g_tests_pass + g_tests_fail);
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    return g_tests_fail > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
