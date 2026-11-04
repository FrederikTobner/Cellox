#pragma once

#include "fixtures/chunk_test_base.h"
#include <gtest/gtest.h>
#include <string>

extern "C" {
#include "byte-code/chunk_disassembler.h"
}

/// Base for the fixtures of the chunk disassembler tests. Every test file derives its own fixture from it.
class ChunkDisassemblerTestBase : public ChunkTestBase {
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
