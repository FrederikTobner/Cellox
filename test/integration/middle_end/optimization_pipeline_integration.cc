#include "fixtures/virtual_machine_test_base.h"
#include <gtest/gtest.h>

#include <string>

extern "C" {
#include "backend/garbage_collector.h"
#include "backend/virtual_machine.h"
#include "byte-code/chunk.h"
#include "frontend/compiler.h"
#include "language-models/object.h"
#include "middle-end/optimization_pass.h"
}

/// Compiles real cellox source code (frontend) and checks the bytecode that the optimization passes of the middle end
/// leave behind. Semantic preservation of the optimizations is covered by the e2e tests.
class OptimizationPipelineTest : public VirtualMachineTestBase {
  protected:
    void SetUp() override {
        VirtualMachineTestBase::SetUp();
        garbage_collector_set_mark_roots_hook(compiler_mark_roots);
        previousLevel = optimization_get_level();
    }

    void TearDown() override {
        optimization_set_level(previousLevel);
        optimization_module_cleanup();
        VirtualMachineTestBase::TearDown();
    }

    object_function_t * compileWithLevel(std::string const & source, uint32_t level) {
        optimization_set_level(level);
        object_function_t * function = compiler_compile(source.c_str());
        EXPECT_NE(nullptr, function);
        return function;
    }

    static bool hasNumberConstant(chunk_t const & chunk, double expected) {
        for (uint32_t i = 0; i < chunk.constants.count; i++) {
            value_t constant = chunk.constants.values[i];
            if (IS_NUMBER(constant) && AS_NUMBER(constant) == expected) {
                return true;
            }
        }
        return false;
    }

    static object_function_t * findFunctionConstant(chunk_t const & chunk) {
        for (uint32_t i = 0; i < chunk.constants.count; i++) {
            if (IS_FUNCTION(chunk.constants.values[i])) {
                return AS_FUNCTION(chunk.constants.values[i]);
            }
        }
        return nullptr;
    }

  private:
    uint32_t previousLevel = 0;
};

TEST_F(OptimizationPipelineTest, ConstantArithmeticIsFoldedIntoSingleConstant) {
    std::string source = "printf(\"{}\", 2 * 3 + 4 * 5 - 6 / 2);";

    object_function_t * unoptimized = compileWithLevel(source, 0);
    object_function_t * optimized = compileWithLevel(source, 2);
    ASSERT_NE(nullptr, unoptimized);
    ASSERT_NE(nullptr, optimized);

    EXPECT_LT(optimized->chunk.byteCodeCount, unoptimized->chunk.byteCodeCount);
    EXPECT_TRUE(hasNumberConstant(optimized->chunk, 23.0));
    EXPECT_FALSE(hasNumberConstant(unoptimized->chunk, 23.0));
}

TEST_F(OptimizationPipelineTest, NestedExpressionsAreFoldedRecursively) {
    std::string source = "printf(\"{}\", (2 + 3) * (4 + 1));";

    object_function_t * optimized = compileWithLevel(source, 2);
    ASSERT_NE(nullptr, optimized);

    EXPECT_TRUE(hasNumberConstant(optimized->chunk, 25.0));
}

TEST_F(OptimizationPipelineTest, ExpressionsWithVariablesAreNotFolded) {
    std::string source = "var x = 5;\n"
                         "var y = 3;\n"
                         "printf(\"{}\", x + y);\n";

    object_function_t * unoptimized = compileWithLevel(source, 0);
    object_function_t * optimized = compileWithLevel(source, 2);
    ASSERT_NE(nullptr, unoptimized);
    ASSERT_NE(nullptr, optimized);

    EXPECT_EQ(unoptimized->chunk.byteCodeCount, optimized->chunk.byteCodeCount);
    EXPECT_FALSE(hasNumberConstant(optimized->chunk, 8.0));
}

TEST_F(OptimizationPipelineTest, DivisionByZeroIsNotFolded) {
    std::string source = "printf(\"{}\", 1 / 0);";

    object_function_t * unoptimized = compileWithLevel(source, 0);
    object_function_t * optimized = compileWithLevel(source, 2);
    ASSERT_NE(nullptr, unoptimized);
    ASSERT_NE(nullptr, optimized);

    EXPECT_EQ(unoptimized->chunk.byteCodeCount, optimized->chunk.byteCodeCount);
}

TEST_F(OptimizationPipelineTest, AlgebraicIdentitiesAreRemoved) {
    std::string source = "printf(\"{}\", 42 + 0);\n"
                         "printf(\"{}\", 42 - 0);\n"
                         "printf(\"{}\", 7 * 1);\n"
                         "printf(\"{}\", 84 / 1);\n";

    object_function_t * unoptimized = compileWithLevel(source, 0);
    object_function_t * optimized = compileWithLevel(source, 2);
    ASSERT_NE(nullptr, unoptimized);
    ASSERT_NE(nullptr, optimized);

    EXPECT_LT(optimized->chunk.byteCodeCount, unoptimized->chunk.byteCodeCount);
}

TEST_F(OptimizationPipelineTest, ConstantConditionBranchesAreShortened) {
    std::string source = "if (true) { printf(\"{}\", 1); } else { printf(\"{}\", 2); }\n";

    object_function_t * unoptimized = compileWithLevel(source, 0);
    object_function_t * optimized = compileWithLevel(source, 2);
    ASSERT_NE(nullptr, unoptimized);
    ASSERT_NE(nullptr, optimized);

    EXPECT_LT(optimized->chunk.byteCodeCount, unoptimized->chunk.byteCodeCount);
}

TEST_F(OptimizationPipelineTest, CodeAfterReturnIsEliminated) {
    std::string source = "fun compute() {\n"
                         "    return 11;\n"
                         "    printf(\"{}\", 999);\n"
                         "}\n";

    object_function_t * unoptimized = compileWithLevel(source, 0);
    object_function_t * optimized = compileWithLevel(source, 2);
    ASSERT_NE(nullptr, unoptimized);
    ASSERT_NE(nullptr, optimized);

    object_function_t * unoptimizedFunction = findFunctionConstant(unoptimized->chunk);
    object_function_t * optimizedFunction = findFunctionConstant(optimized->chunk);
    ASSERT_NE(nullptr, unoptimizedFunction);
    ASSERT_NE(nullptr, optimizedFunction);

    EXPECT_LT(optimizedFunction->chunk.byteCodeCount, unoptimizedFunction->chunk.byteCodeCount);
}

TEST_F(OptimizationPipelineTest, LevelZeroLeavesBytecodeUntouched) {
    std::string source = "printf(\"{}\", 1 + 2);";

    object_function_t * unoptimized = compileWithLevel(source, 0);
    ASSERT_NE(nullptr, unoptimized);

    EXPECT_FALSE(hasNumberConstant(unoptimized->chunk, 3.0));
}
