#include <gtest/gtest.h>

#include "../../fixtures/chunk_fixture.h"
#include <string>

extern "C" {
#include "byte-code/chunk.h"
#include "byte-code/chunk_disassembler.h"
}

class ChunkDisassemblerSimple : public ChunkDisassemblerTest {};

TEST_F(ChunkDisassemblerSimple, AddReturnsNextOffset) {
    // Arrange
    write1(OP_ADD, 1);
    // Act
    int32_t next = disassembleInstruction(0);
    // Assert
    EXPECT_EQ(1, next);
}

TEST_F(ChunkDisassemblerSimple, AddOutputContainsName) {
    // Arrange
    write1(OP_ADD, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("ADD"));
}

TEST_F(ChunkDisassemblerSimple, SubtractReturnsNextOffset) {
    // Arrange
    write1(OP_SUBTRACT, 1);
    // Act
    int32_t next = disassembleInstruction(0);
    // Assert
    EXPECT_EQ(1, next);
}

TEST_F(ChunkDisassemblerSimple, MultiplyReturnsNextOffset) {
    // Arrange
    write1(OP_MULTIPLY, 1);
    // Act
    int32_t next = disassembleInstruction(0);
    // Assert
    EXPECT_EQ(1, next);
}

TEST_F(ChunkDisassemblerSimple, DivideReturnsNextOffset) {
    // Arrange
    write1(OP_DIVIDE, 1);
    // Act
    int32_t next = disassembleInstruction(0);
    // Assert
    EXPECT_EQ(1, next);
}

TEST_F(ChunkDisassemblerSimple, DupOutputContainsName) {
    // Arrange
    write1(OP_DUP, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("DUP"));
}

TEST_F(ChunkDisassemblerSimple, ReturnReturnsNextOffset) {
    // Arrange
    write1(OP_RETURN, 1);
    // Act
    int32_t next = disassembleInstruction(0);
    // Assert
    EXPECT_EQ(1, next);
}

TEST_F(ChunkDisassemblerSimple, NullReturnsNextOffset) {
    // Arrange
    write1(OP_NULL, 1);
    // Act
    int32_t next = disassembleInstruction(0);
    // Assert
    EXPECT_EQ(1, next);
}

TEST_F(ChunkDisassemblerSimple, TrueOutputContainsName) {
    // Arrange
    write1(OP_TRUE, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("TRUE"));
}

TEST_F(ChunkDisassemblerSimple, FalseOutputContainsName) {
    // Arrange
    write1(OP_FALSE, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("FALSE"));
}

TEST_F(ChunkDisassemblerSimple, NegateOutputContainsName) {
    // Arrange
    write1(OP_NEGATE, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("NEGATE"));
}

TEST_F(ChunkDisassemblerSimple, NotOutputContainsName) {
    // Arrange
    write1(OP_NOT, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("NOT"));
}

TEST_F(ChunkDisassemblerSimple, EqualOutputContainsName) {
    // Arrange
    write1(OP_EQUAL, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("EQUAL"));
}

TEST_F(ChunkDisassemblerSimple, GreaterOutputContainsName) {
    // Arrange
    write1(OP_GREATER, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("GREATER"));
}

TEST_F(ChunkDisassemblerSimple, LessOutputContainsName) {
    // Arrange
    write1(OP_LESS, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("LESS"));
}

TEST_F(ChunkDisassemblerSimple, PopOutputContainsName) {
    // Arrange
    write1(OP_POP, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("POP"));
}

TEST_F(ChunkDisassemblerSimple, GetSliceOfOutputContainsName) {
    // Arrange
    write1(OP_GET_SLICE_OF, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("GET_RANGE_OF"));
}

TEST_F(ChunkDisassemblerSimple, InheritOutputContainsName) {
    // Arrange
    write1(OP_INHERIT, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("INHERIT"));
}

TEST_F(ChunkDisassemblerSimple, UnknownOpcodeReturnsOffsetPlusOne) {
    // 0xFF is not a valid opcode
    // Arrange
    write1(0xFF, 1);
    // Act
    int32_t next = disassembleInstruction(0);
    // Assert
    EXPECT_EQ(1, next);
}

TEST_F(ChunkDisassemblerSimple, UnknownOpcodeOutputContainsUnknown) {
    // Arrange
    write1(0xFF, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("Unknown opcode"));
}

TEST_F(ChunkDisassemblerSimple, SameLineShowsPipeMarker) {
    // Arrange
    write1(OP_ADD, 5);
    write1(OP_RETURN, 5);
    // Act
    std::string out = captureDisassembleInstruction(1); // offset 1 shares line with offset 0
    // Assert
    EXPECT_NE(std::string::npos, out.find("|"));
}
