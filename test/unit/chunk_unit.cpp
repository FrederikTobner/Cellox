#include "fixtures/chunk_test_base.h"
#include <gtest/gtest.h>

extern "C" {
#include "byte-code/chunk.h"
}

class ChunkTest : public ChunkWithVirtualMachineTestBase {};

TEST_F(ChunkTest, DetermineLineByInstructionIndex) {
    // Arrange
    chunk_write(&chunk, OP_CONSTANT, 1);
    chunk_write(&chunk, 0, 1);
    chunk_write(&chunk, OP_RETURN, 2);

    // Assert
    EXPECT_EQ(1u, chunk_determine_line_by_index(&chunk, 0));
    EXPECT_EQ(1u, chunk_determine_line_by_index(&chunk, 1));
    EXPECT_EQ(2u, chunk_determine_line_by_index(&chunk, 2));
}

TEST_F(ChunkTest, AddConstantReturnsSequentialIndices) {
    // Act
    int32_t first = chunk_add_constant(&chunk, NUMBER_VAL(12));
    int32_t second = chunk_add_constant(&chunk, NUMBER_VAL(34));

    // Assert
    EXPECT_EQ(0, first);
    EXPECT_EQ(1, second);
    ASSERT_EQ(2u, chunk.constants.count);
    EXPECT_DOUBLE_EQ(12.0, AS_NUMBER(chunk.constants.values[0]));
    EXPECT_DOUBLE_EQ(34.0, AS_NUMBER(chunk.constants.values[1]));
}
