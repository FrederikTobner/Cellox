#pragma once

#include <gtest/gtest.h>

extern "C" {
#include "backend/virtual_machine.h"
#include "byte-code/chunk.h"
}

/// Base for the fixtures of tests that need a chunk. Not meant to be used directly with TEST_F, every test file
/// derives its own fixture that is named after the unit that is tested.
class ChunkTestBase : public ::testing::Test {
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

/// Base for the fixtures of tests that need a chunk and a running virtual machine (e.g. for allocating objects).
class ChunkWithVirtualMachineTestBase : public ChunkTestBase {
  protected:
    void SetUp() override {
        virtual_machine_init();
        ChunkTestBase::SetUp();
    }

    void TearDown() override {
        ChunkTestBase::TearDown();
        virtual_machine_free();
    }
};
