#include <gtest/gtest.h>

#include <string>
#include "../../fixtures/chunk_fixture.h"

extern "C" {
#include "byte-code/chunk.h"
#include "byte-code/chunk_disassembler.h"
#include "language-models/value.h"
}

class ChunkDisassemblerConstant : public ChunkDisassemblerTest {};

TEST_F(ChunkDisassemblerConstant, ConstantReturnsOffsetPlusTwo) {
    // Arrange.
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(3.14));
    write2(OP_CONSTANT, (uint8_t)idx, 1);
    // Act
    int32_t next = disassembleInstruction(0);
    // Assert.
    EXPECT_EQ(2, next);
}

TEST_F(ChunkDisassemblerConstant, ConstantOutputContainsName) {
    // Arrange
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(42.0));
    write2(OP_CONSTANT, (uint8_t)idx, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("CONSTANT"));
}

TEST_F(ChunkDisassemblerConstant, DefineGlobalOutputContainsName) {
    // Arrange
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write2(OP_DEFINE_GLOBAL, (uint8_t)idx, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("DEFINE_GLOBAL"));
}

TEST_F(ChunkDisassemblerConstant, GetGlobalOutputContainsName) {
    // Arrange
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write2(OP_GET_GLOBAL, (uint8_t)idx, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert.
    EXPECT_NE(std::string::npos, out.find("GET_GLOBAL"));
}

TEST_F(ChunkDisassemblerConstant, SetGlobalOutputContainsName) {
    // Arrange
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write2(OP_SET_GLOBAL, (uint8_t)idx, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("SET_GLOBAL"));
}

TEST_F(ChunkDisassemblerConstant, GetPropertyOutputContainsName) {
    // Arrange
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write2(OP_GET_PROPERTY, (uint8_t)idx, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("GET_PROPERTY"));
}

TEST_F(ChunkDisassemblerConstant, SetPropertyOutputContainsName) {
    // Arrange
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write2(OP_SET_PROPERTY, (uint8_t)idx, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("SET_PROPERTY"));
}
