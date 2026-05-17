#include <gtest/gtest.h>

#include <string>
#include "../../fixtures/chunk_fixture.h"

extern "C" {
#include "byte-code/chunk.h"
#include "byte-code/chunk_disassembler.h"
#include "language-models/value.h"
}

class ChunkDisassemblerInvoke : public ChunkDisassemblerTest {};

TEST_F(ChunkDisassemblerInvoke, InvokeReturnsOffsetPlusThree) {
    // Arrange
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write3(OP_INVOKE, (uint8_t)idx, 2, 1);
    // Act
    int32_t next = disassembleInstruction(0);
    // Assert.
    EXPECT_EQ(3, next);
}

TEST_F(ChunkDisassemblerInvoke, InvokeOutputContainsName) {
    // Arrange
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write3(OP_INVOKE, (uint8_t)idx, 1, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert.
    EXPECT_NE(std::string::npos, out.find("INVOKE"));
}

TEST_F(ChunkDisassemblerInvoke, SuperInvokeReturnsOffsetPlusThree) {
    // Arrange
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write3(OP_SUPER_INVOKE, (uint8_t)idx, 0, 1);
    // Act
    int32_t next = disassembleInstruction(0);
    // Assert
    EXPECT_EQ(3, next);
}

TEST_F(ChunkDisassemblerInvoke, SuperInvokeOutputContainsName) {
    // Arrange
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write3(OP_SUPER_INVOKE, (uint8_t)idx, 0, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("SUPER_INVOKE"));
}
