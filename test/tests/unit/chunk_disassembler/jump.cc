#include <gtest/gtest.h>

#include "../../fixtures/chunk_fixture.h"
#include <string>

extern "C" {
#include "byte-code/chunk.h"
#include "byte-code/chunk_disassembler.h"
}

class ChunkDisassemblerJump : public ChunkDisassemblerTest {};

TEST_F(ChunkDisassemblerJump, JumpReturnsOffsetPlusThree) {
    // Arrange
    write3(OP_JUMP, 0x00, 0x05, 1);
    // Act
    int32_t next = disassembleInstruction(0);
    // Assert
    EXPECT_EQ(3, next);
}

TEST_F(ChunkDisassemblerJump, JumpOutputContainsName) {
    // Arrange
    write3(OP_JUMP, 0x00, 0x04, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("JUMP"));
}

TEST_F(ChunkDisassemblerJump, JumpIfFalseReturnsOffsetPlusThree) {
    // Arrange
    write3(OP_JUMP_IF_FALSE, 0x00, 0x08, 1);
    // Act
    int32_t next = disassembleInstruction(0);
    // Assert
    EXPECT_EQ(3, next);
}

TEST_F(ChunkDisassemblerJump, JumpIfFalseOutputContainsName) {
    // Arrange
    write3(OP_JUMP_IF_FALSE, 0x00, 0x08, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("JUMP_IF_FALSE"));
}

TEST_F(ChunkDisassemblerJump, LoopReturnsOffsetPlusThree) {
    // Arrange
    write3(OP_LOOP, 0x00, 0x03, 1);
    // Act
    int32_t next = disassembleInstruction(0);
    // Assert
    EXPECT_EQ(3, next);
}

TEST_F(ChunkDisassemblerJump, LoopOutputContainsName) {
    // Arrange
    write3(OP_LOOP, 0x00, 0x03, 1);
    // Act
    std::string out = captureDisassembleInstruction(0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("LOOP"));
}
