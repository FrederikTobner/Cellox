#pragma once

#include "vm_fixture.h"
#include <gtest/gtest.h>
#include <string>

extern "C" {
#include "backend/virtual_machine.h"
#include "byte-code/chunk.h"
#include "byte-code/chunk_disassembler.h"
}

class ChunkFixture : public ::testing::Test {
  protected:
    chunk_t chunk;

    void SetUp() override {
        chunk_init(&chunk);
    }

    void TearDown() override {
        chunk_free(&chunk);
    }

    void write1(uint8_t op, uint32_t line) {
        chunk_write(&chunk, op, line);
    }

    void write2(uint8_t op, uint8_t operand, uint32_t line) {
        chunk_write(&chunk, op, line);
        chunk_write(&chunk, operand, line);
    }

    void write3(uint8_t op, uint8_t b1, uint8_t b2, uint32_t line) {
        chunk_write(&chunk, op, line);
        chunk_write(&chunk, b1, line);
        chunk_write(&chunk, b2, line);
    }

    void emitConstant(uint8_t constant_index, int32_t line = 1) {
        write2(OP_CONSTANT, constant_index, line);
    }

    void emitSimple(uint8_t opcode, int32_t line = 1) {
        write1(opcode, line);
    }
};

class ChunkVMFixture : public ChunkFixture {
  protected:
    void SetUp() override {
        virtual_machine_init();
        ChunkFixture::SetUp();
    }

    void TearDown() override {
        ChunkFixture::TearDown();
        virtual_machine_free();
    }
};

class ChunkVMTest : public ChunkVMFixture {};

class ChunkDisassemblerTest : public ChunkFixture {
  protected:
    std::string captureDisassembleInstruction(int32_t offset) {
        testing::internal::CaptureStdout();
        chunk_disassembler_disassemble_instruction(&chunk, offset);
        return testing::internal::GetCapturedStdout();
    }

    int32_t disassembleInstruction(int32_t offset) {
        testing::internal::CaptureStdout();
        int32_t next = chunk_disassembler_disassemble_instruction(&chunk, offset);
        testing::internal::GetCapturedStdout();
        return next;
    }

    std::string captureDisassembleChunk(const char * name, int32_t offset) {
        testing::internal::CaptureStdout();
        chunk_disassembler_disassemble_chunk(&chunk, name, offset);
        return testing::internal::GetCapturedStdout();
    }
};
