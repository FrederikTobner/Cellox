#include <gtest/gtest.h>

#include <string>
#include "../../fixtures/chunk_fixture.h"

extern "C" {
#include "byte-code/chunk.h"
#include "byte-code/chunk_disassembler.h"
}

class ChunkDisassemblerByte : public ChunkDisassemblerTest {};

TEST_F(ChunkDisassemblerByte, CallReturnsOffsetPlusTwo) {
    // Arrange.
    write2(OP_CALL, 2, 1);
    // Act.
    int32_t next = disassembleInstruction(0);
    // Assert.
    EXPECT_EQ(2, next);
}

TEST_F(ChunkDisassemblerByte, CallOutputContainsName) {
    // Arrange
    write2(OP_CALL, 3, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("CALL"));
}

TEST_F(ChunkDisassemblerByte, GetLocalOutputContainsName) {
    // Arrange
    write2(OP_GET_LOCAL, 0, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("GET_LOCAL"));
}

TEST_F(ChunkDisassemblerByte, SetLocalOutputContainsName) {
    // Arrange
    write2(OP_SET_LOCAL, 1, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("SET_LOCAL"));
}

TEST_F(ChunkDisassemblerByte, GetUpvalueOutputContainsName) {
    // Arrange
    write2(OP_GET_UPVALUE, 0, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("GET_UPVALUE"));
}

TEST_F(ChunkDisassemblerByte, SetUpvalueOutputContainsName) {
    // Arrange
    write2(OP_SET_UPVALUE, 0, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("SET_UPVALUE"));
}

TEST_F(ChunkDisassemblerByte, ArrayLiteralOutputContainsName) {
    // Arrange.
    write2(OP_ARRAY_LITERAL, 3, 1);
    // Act.
    std::string out = captureDisassembleInstruction(0);
    // Assert.
    EXPECT_NE(std::string::npos, out.find("DYNAMIC_ARRAY_LITERAL"));
}
