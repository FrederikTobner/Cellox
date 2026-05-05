#include <gtest/gtest.h>

#include <string>

extern "C" {
#include "byte-code/chunk.h"
#include "byte-code/chunk_disassembler.h"
#include "language-models/value.h"
}

// ── helpers ──────────────────────────────────────────────────────────────────

/// Write a single-byte instruction and a matching line-info entry.
static void write1(chunk_t * c, uint8_t op, uint32_t line) {
    chunk_write(c, op, line);
}

/// Write a two-byte instruction (opcode + one operand byte).
static void write2(chunk_t * c, uint8_t op, uint8_t operand, uint32_t line) {
    chunk_write(c, op, line);
    chunk_write(c, operand, line);
}

/// Write a three-byte instruction (opcode + two operand bytes).
static void write3(chunk_t * c, uint8_t op, uint8_t b1, uint8_t b2, uint32_t line) {
    chunk_write(c, op, line);
    chunk_write(c, b1, line);
    chunk_write(c, b2, line);
}

// ── simple instructions ───────────────────────────────────────────────────────

class ChunkDisassemblerSimple : public ::testing::Test {
  protected:
    chunk_t chunk;
    void SetUp() override { chunk_init(&chunk); }
    void TearDown() override { chunk_free(&chunk); }
};

TEST_F(ChunkDisassemblerSimple, AddReturnsNextOffset) {
    write1(&chunk, OP_ADD, 1);
    testing::internal::CaptureStdout();
    int32_t next = chunk_disassembler_disassemble_instruction(&chunk, 0);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(1, next);
}

TEST_F(ChunkDisassemblerSimple, AddOutputContainsName) {
    write1(&chunk, OP_ADD, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("ADD"));
}

TEST_F(ChunkDisassemblerSimple, SubtractReturnsNextOffset) {
    write1(&chunk, OP_SUBTRACT, 1);
    testing::internal::CaptureStdout();
    int32_t next = chunk_disassembler_disassemble_instruction(&chunk, 0);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(1, next);
}

TEST_F(ChunkDisassemblerSimple, MultiplyReturnsNextOffset) {
    write1(&chunk, OP_MULTIPLY, 1);
    testing::internal::CaptureStdout();
    int32_t next = chunk_disassembler_disassemble_instruction(&chunk, 0);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(1, next);
}

TEST_F(ChunkDisassemblerSimple, DivideReturnsNextOffset) {
    write1(&chunk, OP_DIVIDE, 1);
    testing::internal::CaptureStdout();
    int32_t next = chunk_disassembler_disassemble_instruction(&chunk, 0);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(1, next);
}

TEST_F(ChunkDisassemblerSimple, DupOutputContainsName) {
    write1(&chunk, OP_DUP, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("DUP"));
}

TEST_F(ChunkDisassemblerSimple, ReturnReturnsNextOffset) {
    write1(&chunk, OP_RETURN, 1);
    testing::internal::CaptureStdout();
    int32_t next = chunk_disassembler_disassemble_instruction(&chunk, 0);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(1, next);
}

TEST_F(ChunkDisassemblerSimple, NullReturnsNextOffset) {
    write1(&chunk, OP_NULL, 1);
    testing::internal::CaptureStdout();
    int32_t next = chunk_disassembler_disassemble_instruction(&chunk, 0);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(1, next);
}

TEST_F(ChunkDisassemblerSimple, TrueOutputContainsName) {
    write1(&chunk, OP_TRUE, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("TRUE"));
}

TEST_F(ChunkDisassemblerSimple, FalseOutputContainsName) {
    write1(&chunk, OP_FALSE, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("FALSE"));
}

TEST_F(ChunkDisassemblerSimple, NegateOutputContainsName) {
    write1(&chunk, OP_NEGATE, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("NEGATE"));
}

TEST_F(ChunkDisassemblerSimple, NotOutputContainsName) {
    write1(&chunk, OP_NOT, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("NOT"));
}

TEST_F(ChunkDisassemblerSimple, EqualOutputContainsName) {
    write1(&chunk, OP_EQUAL, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("EQUAL"));
}

TEST_F(ChunkDisassemblerSimple, GreaterOutputContainsName) {
    write1(&chunk, OP_GREATER, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("GREATER"));
}

TEST_F(ChunkDisassemblerSimple, LessOutputContainsName) {
    write1(&chunk, OP_LESS, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("LESS"));
}

TEST_F(ChunkDisassemblerSimple, PopOutputContainsName) {
    write1(&chunk, OP_POP, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("POP"));
}

TEST_F(ChunkDisassemblerSimple, GetSliceOfOutputContainsName) {
    write1(&chunk, OP_GET_SLICE_OF, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("GET_RANGE_OF"));
}

TEST_F(ChunkDisassemblerSimple, InheritOutputContainsName) {
    write1(&chunk, OP_INHERIT, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("INHERIT"));
}

TEST_F(ChunkDisassemblerSimple, UnknownOpcodeReturnsOffsetPlusOne) {
    // 0xFF is not a valid opcode
    write1(&chunk, 0xFF, 1);
    testing::internal::CaptureStdout();
    int32_t next = chunk_disassembler_disassemble_instruction(&chunk, 0);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(1, next);
}

TEST_F(ChunkDisassemblerSimple, UnknownOpcodeOutputContainsUnknown) {
    write1(&chunk, 0xFF, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("Unknown opcode"));
}

// ── same-line pipe marker ─────────────────────────────────────────────────────

TEST_F(ChunkDisassemblerSimple, SameLineShowsPipeMarker) {
    // Both instructions on line 5 — the second should print "   | " instead of
    // the line number.
    write1(&chunk, OP_ADD, 5);
    write1(&chunk, OP_RETURN, 5);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 1); // offset 1 shares line with offset 0
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("|"));
}

// ── byte instructions ─────────────────────────────────────────────────────────

class ChunkDisassemblerByte : public ::testing::Test {
  protected:
    chunk_t chunk;
    void SetUp() override { chunk_init(&chunk); }
    void TearDown() override { chunk_free(&chunk); }
};

TEST_F(ChunkDisassemblerByte, CallReturnsOffsetPlusTwo) {
    write2(&chunk, OP_CALL, 2, 1);
    testing::internal::CaptureStdout();
    int32_t next = chunk_disassembler_disassemble_instruction(&chunk, 0);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(2, next);
}

TEST_F(ChunkDisassemblerByte, CallOutputContainsName) {
    write2(&chunk, OP_CALL, 3, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("CALL"));
}

TEST_F(ChunkDisassemblerByte, GetLocalOutputContainsName) {
    write2(&chunk, OP_GET_LOCAL, 0, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("GET_LOCAL"));
}

TEST_F(ChunkDisassemblerByte, SetLocalOutputContainsName) {
    write2(&chunk, OP_SET_LOCAL, 1, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("SET_LOCAL"));
}

TEST_F(ChunkDisassemblerByte, GetUpvalueOutputContainsName) {
    write2(&chunk, OP_GET_UPVALUE, 0, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("GET_UPVALUE"));
}

TEST_F(ChunkDisassemblerByte, SetUpvalueOutputContainsName) {
    write2(&chunk, OP_SET_UPVALUE, 0, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("SET_UPVALUE"));
}

TEST_F(ChunkDisassemblerByte, ArrayLiteralOutputContainsName) {
    write2(&chunk, OP_ARRAY_LITERAL, 3, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("DYNAMIC_ARRAY_LITERAL"));
}

// ── constant instructions ─────────────────────────────────────────────────────

class ChunkDisassemblerConstant : public ::testing::Test {
  protected:
    chunk_t chunk;
    void SetUp() override { chunk_init(&chunk); }
    void TearDown() override { chunk_free(&chunk); }
};

TEST_F(ChunkDisassemblerConstant, ConstantReturnsOffsetPlusTwo) {
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(3.14));
    write2(&chunk, OP_CONSTANT, (uint8_t)idx, 1);
    testing::internal::CaptureStdout();
    int32_t next = chunk_disassembler_disassemble_instruction(&chunk, 0);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(2, next);
}

TEST_F(ChunkDisassemblerConstant, ConstantOutputContainsName) {
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(42.0));
    write2(&chunk, OP_CONSTANT, (uint8_t)idx, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("CONSTANT"));
}

TEST_F(ChunkDisassemblerConstant, DefineGlobalOutputContainsName) {
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write2(&chunk, OP_DEFINE_GLOBAL, (uint8_t)idx, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("DEFINE_GLOBAL"));
}

TEST_F(ChunkDisassemblerConstant, GetGlobalOutputContainsName) {
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write2(&chunk, OP_GET_GLOBAL, (uint8_t)idx, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("GET_GLOBAL"));
}

TEST_F(ChunkDisassemblerConstant, SetGlobalOutputContainsName) {
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write2(&chunk, OP_SET_GLOBAL, (uint8_t)idx, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("SET_GLOBAL"));
}

TEST_F(ChunkDisassemblerConstant, GetPropertyOutputContainsName) {
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write2(&chunk, OP_GET_PROPERTY, (uint8_t)idx, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("GET_PROPERTY"));
}

TEST_F(ChunkDisassemblerConstant, SetPropertyOutputContainsName) {
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write2(&chunk, OP_SET_PROPERTY, (uint8_t)idx, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("SET_PROPERTY"));
}

// ── jump instructions ─────────────────────────────────────────────────────────

class ChunkDisassemblerJump : public ::testing::Test {
  protected:
    chunk_t chunk;
    void SetUp() override { chunk_init(&chunk); }
    void TearDown() override { chunk_free(&chunk); }
};

TEST_F(ChunkDisassemblerJump, JumpReturnsOffsetPlusThree) {
    write3(&chunk, OP_JUMP, 0x00, 0x05, 1);
    testing::internal::CaptureStdout();
    int32_t next = chunk_disassembler_disassemble_instruction(&chunk, 0);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(3, next);
}

TEST_F(ChunkDisassemblerJump, JumpOutputContainsName) {
    write3(&chunk, OP_JUMP, 0x00, 0x04, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("JUMP"));
}

TEST_F(ChunkDisassemblerJump, JumpIfFalseReturnsOffsetPlusThree) {
    write3(&chunk, OP_JUMP_IF_FALSE, 0x00, 0x08, 1);
    testing::internal::CaptureStdout();
    int32_t next = chunk_disassembler_disassemble_instruction(&chunk, 0);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(3, next);
}

TEST_F(ChunkDisassemblerJump, JumpIfFalseOutputContainsName) {
    write3(&chunk, OP_JUMP_IF_FALSE, 0x00, 0x08, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("JUMP_IF_FALSE"));
}

TEST_F(ChunkDisassemblerJump, LoopReturnsOffsetPlusThree) {
    write3(&chunk, OP_LOOP, 0x00, 0x03, 1);
    testing::internal::CaptureStdout();
    int32_t next = chunk_disassembler_disassemble_instruction(&chunk, 0);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(3, next);
}

TEST_F(ChunkDisassemblerJump, LoopOutputContainsName) {
    write3(&chunk, OP_LOOP, 0x00, 0x03, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("LOOP"));
}

// ── invoke instructions ───────────────────────────────────────────────────────

class ChunkDisassemblerInvoke : public ::testing::Test {
  protected:
    chunk_t chunk;
    void SetUp() override { chunk_init(&chunk); }
    void TearDown() override { chunk_free(&chunk); }
};

TEST_F(ChunkDisassemblerInvoke, InvokeReturnsOffsetPlusThree) {
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write3(&chunk, OP_INVOKE, (uint8_t)idx, 2, 1);
    testing::internal::CaptureStdout();
    int32_t next = chunk_disassembler_disassemble_instruction(&chunk, 0);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(3, next);
}

TEST_F(ChunkDisassemblerInvoke, InvokeOutputContainsName) {
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write3(&chunk, OP_INVOKE, (uint8_t)idx, 1, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("INVOKE"));
}

TEST_F(ChunkDisassemblerInvoke, SuperInvokeReturnsOffsetPlusThree) {
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write3(&chunk, OP_SUPER_INVOKE, (uint8_t)idx, 0, 1);
    testing::internal::CaptureStdout();
    int32_t next = chunk_disassembler_disassemble_instruction(&chunk, 0);
    testing::internal::GetCapturedStdout();
    EXPECT_EQ(3, next);
}

TEST_F(ChunkDisassemblerInvoke, SuperInvokeOutputContainsName) {
    int32_t idx = chunk_add_constant(&chunk, NUMBER_VAL(0));
    write3(&chunk, OP_SUPER_INVOKE, (uint8_t)idx, 0, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_instruction(&chunk, 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("SUPER_INVOKE"));
}

// ── disassemble_chunk ─────────────────────────────────────────────────────────

class ChunkDisassemblerChunk : public ::testing::Test {
  protected:
    chunk_t chunk;
    void SetUp() override { chunk_init(&chunk); }
    void TearDown() override { chunk_free(&chunk); }
};

TEST_F(ChunkDisassemblerChunk, OutputContainsFunctionName) {
    write1(&chunk, OP_RETURN, 1);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_chunk(&chunk, "myFunc", 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("myFunc"));
}

TEST_F(ChunkDisassemblerChunk, AllInstructionsDisassembled) {
    write1(&chunk, OP_TRUE, 1);
    write1(&chunk, OP_NOT, 1);
    write1(&chunk, OP_RETURN, 2);
    testing::internal::CaptureStdout();
    chunk_disassembler_disassemble_chunk(&chunk, "test", 0);
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, out.find("TRUE"));
    EXPECT_NE(std::string::npos, out.find("NOT"));
    EXPECT_NE(std::string::npos, out.find("RETURN"));
}
