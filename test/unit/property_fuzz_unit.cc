#include <gtest/gtest.h>

#include <random>
#include <vector>

extern "C" {
#include "byte-code/chunk.h"
#include "language-models/dynamic_value_array.h"
}

TEST(PropertyFuzzUnit, DynamicArrayWriteRemoveMatchesModel) {
    // Arrange
    std::mt19937 rng(1337u);

    for (int round = 0; round < 50; round++) {
        dynamic_value_array_t array;
        dynamic_value_array_init(&array);
        std::vector<int> model;

        std::uniform_int_distribution<int> opDist(0, 1);
        std::uniform_int_distribution<int> valueDist(-1000, 1000);

        for (int step = 0; step < 200; step++) {
            bool doWrite = model.empty() || opDist(rng) == 0;
            if (doWrite) {
                int value = valueDist(rng);
                model.push_back(value);
                // Act
                dynamic_value_array_write(&array, NUMBER_VAL(value));
            } else {
                std::uniform_int_distribution<size_t> idxDist(0, model.size() - 1);
                size_t index = idxDist(rng);
                model.erase(model.begin() + static_cast<long>(index));
                dynamic_value_array_remove(&array, index);
            }

            // Assert
            ASSERT_EQ(model.size(), array.count);
            for (size_t i = 0; i < model.size(); i++) {
                EXPECT_DOUBLE_EQ(model[i], AS_NUMBER(array.values[i]));
            }
        }

        dynamic_value_array_free(&array);
    }
}

TEST(PropertyFuzzUnit, ChunkLineLookupMatchesRecordedLines) {
    // Arrange
    std::mt19937 rng(424242u);

    for (int round = 0; round < 30; round++) {
        chunk_t chunk;
        // Act
        chunk_init(&chunk);
        std::vector<uint32_t> expectedLines;

        std::uniform_int_distribution<int> lineJumpDist(0, 2);
        uint32_t line = 1;

        for (int i = 0; i < 250; i++) {
            line += static_cast<uint32_t>(lineJumpDist(rng));
            uint8_t byte = (i % 2 == 0) ? OP_CONSTANT : OP_RETURN;
            chunk_write(&chunk, byte, static_cast<int32_t>(line));
            expectedLines.push_back(line);
        }

        for (uint32_t i = 0; i < chunk.byteCodeCount; i++) {
            // Assert
            EXPECT_EQ(expectedLines[i], chunk_determine_line_by_index(&chunk, i));
        }

        chunk_free(&chunk);
    }
}
