#include <gtest/gtest.h>

extern "C" {
#include "backend/virtual_machine.h"
#include "byte-code/chunk.h"
#include "language-models/data-structures/dynamic_value_array.h"
}

TEST(DynamicValueArrayUnit, RemoveMiddleElementShiftsTail) {
    dynamic_value_array_t array;
    dynamic_value_array_init(&array);

    dynamic_value_array_write(&array, NUMBER_VAL(1));
    dynamic_value_array_write(&array, NUMBER_VAL(2));
    dynamic_value_array_write(&array, NUMBER_VAL(3));

    dynamic_value_array_remove(&array, 1);

    ASSERT_EQ(2u, array.count);
    EXPECT_DOUBLE_EQ(1.0, AS_NUMBER(array.values[0]));
    EXPECT_DOUBLE_EQ(3.0, AS_NUMBER(array.values[1]));

    dynamic_value_array_free(&array);
}

TEST(DynamicValueArrayUnit, RemoveOutOfBoundsKeepsArrayUnchanged) {
    dynamic_value_array_t array;
    dynamic_value_array_init(&array);

    dynamic_value_array_write(&array, NUMBER_VAL(7));
    dynamic_value_array_remove(&array, 5);

    ASSERT_EQ(1u, array.count);
    EXPECT_DOUBLE_EQ(7.0, AS_NUMBER(array.values[0]));

    dynamic_value_array_free(&array);
}

TEST(ChunkUnit, DetermineLineByInstructionIndex) {
    chunk_t chunk;
    chunk_init(&chunk);

    chunk_write(&chunk, OP_CONSTANT, 1);
    chunk_write(&chunk, 0, 1);
    chunk_write(&chunk, OP_RETURN, 2);

    EXPECT_EQ(1u, chunk_determine_line_by_index(&chunk, 0));
    EXPECT_EQ(1u, chunk_determine_line_by_index(&chunk, 1));
    EXPECT_EQ(2u, chunk_determine_line_by_index(&chunk, 2));

    chunk_free(&chunk);
}

TEST(ChunkUnit, AddConstantReturnsSequentialIndices) {
    virtual_machine_init();

    chunk_t chunk;
    chunk_init(&chunk);

    int32_t first = chunk_add_constant(&chunk, NUMBER_VAL(12));
    int32_t second = chunk_add_constant(&chunk, NUMBER_VAL(34));

    EXPECT_EQ(0, first);
    EXPECT_EQ(1, second);
    ASSERT_EQ(2u, chunk.constants.count);
    EXPECT_DOUBLE_EQ(12.0, AS_NUMBER(chunk.constants.values[0]));
    EXPECT_DOUBLE_EQ(34.0, AS_NUMBER(chunk.constants.values[1]));

    chunk_free(&chunk);
    virtual_machine_free();
}
