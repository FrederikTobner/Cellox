#include <gtest/gtest.h>

#include "../../fixtures/chunk_fixture.h"
#include <string>

extern "C" {
#include "byte-code/chunk.h"
#include "byte-code/chunk_disassembler.h"
}

class ChunkDisassemblerChunk : public ChunkDisassemblerTest {};

TEST_F(ChunkDisassemblerChunk, OutputContainsFunctionName) {
    // Arrange
    write1(OP_RETURN, 1);
    // Act
    std::string out = captureDisassembleChunk("myFunc", 0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("myFunc"));
}

TEST_F(ChunkDisassemblerChunk, AllInstructionsDisassembled) {
    // Arrange
    write1(OP_TRUE, 1);
    write1(OP_NOT, 1);
    write1(OP_RETURN, 2);
    // Act
    std::string out = captureDisassembleChunk("test", 0);
    // Assert
    EXPECT_NE(std::string::npos, out.find("TRUE"));
    EXPECT_NE(std::string::npos, out.find("NOT"));
    EXPECT_NE(std::string::npos, out.find("RETURN"));
}
