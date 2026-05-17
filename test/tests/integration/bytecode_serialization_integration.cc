#include <gtest/gtest.h>
#include "../fixtures/vm_fixture.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

extern "C" {
#include "backend/virtual_machine.h"
#include "byte-code/chunk.h"
#include "byte-code/chunk_file.h"
#include "clx_os/temp.h"
#include "frontend/compiler.h"
#include "language-models/object.h"
}

class ChunkFileIntegrationFixture : public VirtualMachineFixture {
protected:
    std::string derive_chunk_path(std::string path) {
        path.replace(path.size() - 3, 3, "cxcf");
        return path;
    }

    std::string make_temp_base_path() {
        char * path = clx_os_temp_make_path("cellox_bytecode_", "");
        EXPECT_NE(nullptr, path);
        std::string result = path ? std::string(path) : std::string();
        std::free(path);
        return result;
    }

    std::string make_temp_program_path() {
        return make_temp_base_path() + ".clx";
    }

    std::string make_temp_chunk_path() {
        return make_temp_base_path() + ".cxcf";
    }
};

TEST_F(ChunkFileIntegrationFixture, RoundTripNestedClosureExecutes) {
    const char * source = R"(fun makeAdder(a) {
  fun inner(b) {
    return a + b;
  }

  return inner;
}

var addTwo = makeAdder(2);
printf("{}\n", addTwo(40));
)";

    std::string programPath = make_temp_program_path();
    std::string chunkPath = derive_chunk_path(programPath);

    object_function_t * function = compiler_compile(source);
    ASSERT_NE(nullptr, function);
    ASSERT_EQ(0, chunk_file_store(function->chunk, programPath.c_str(), static_cast<chunk_file_compile_flag>(0)));

    chunk_t * loaded = chunk_file_load(chunkPath.c_str());
    ASSERT_NE(nullptr, loaded);
    ASSERT_GT(loaded->byteCodeCount, 0u);

    bool hasFunctionConstant = false;
    for (uint32_t i = 0; i < loaded->constants.count; i++) {
        if (IS_FUNCTION(loaded->constants.values[i])) {
            hasFunctionConstant = true;
            break;
        }
    }
    EXPECT_TRUE(hasFunctionConstant);

    testing::internal::CaptureStdout();
    interpret_result result = virtual_machine_run_chunk(*loaded);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(INTERPRET_OK, result);
    EXPECT_EQ("42\n", output);

    free(loaded);

    std::remove(programPath.c_str());
    std::remove(chunkPath.c_str());
}

TEST_F(ChunkFileIntegrationFixture, RoundTripPreservesCodeAndLineInfo) {
    const char * source = R"(var x = 1;
x += 41;
printf("{}\n", x);
)";

    std::string programPath = make_temp_program_path();
    std::string chunkPath = derive_chunk_path(programPath);

    object_function_t * function = compiler_compile(source);
    ASSERT_NE(nullptr, function);

    chunk_t * original = &function->chunk;
    ASSERT_GT(original->byteCodeCount, 0u);
    ASSERT_EQ(0, chunk_file_store(*original, programPath.c_str(), static_cast<chunk_file_compile_flag>(0)));

    chunk_t * loaded = chunk_file_load(chunkPath.c_str());
    ASSERT_NE(nullptr, loaded);

    ASSERT_EQ(original->byteCodeCount, loaded->byteCodeCount);
    ASSERT_EQ(original->lineInfoCount, loaded->lineInfoCount);
    ASSERT_EQ(0, std::memcmp(original->code, loaded->code, original->byteCodeCount));

    testing::internal::CaptureStdout();
    interpret_result result = virtual_machine_run_chunk(*loaded);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(INTERPRET_OK, result);
    EXPECT_EQ("42\n", output);

    free(loaded);

    std::remove(programPath.c_str());
    std::remove(chunkPath.c_str());
}

TEST_F(ChunkFileIntegrationFixture, RoundTripPreservesNegativeAndSpecialNumbers) {
    const char * source = R"(printf("{}\n", -1.5);
printf("{}\n", 0.125);
printf("{}\n", 1000000000.25);
)";

    std::string programPath = make_temp_program_path();
    std::string chunkPath = derive_chunk_path(programPath);

    object_function_t * function = compiler_compile(source);
    ASSERT_NE(nullptr, function);

    ASSERT_EQ(0, chunk_file_store(function->chunk, programPath.c_str(), static_cast<chunk_file_compile_flag>(0)));

    chunk_t * loaded = chunk_file_load(chunkPath.c_str());
    ASSERT_NE(nullptr, loaded);

    testing::internal::CaptureStdout();
    interpret_result result = virtual_machine_run_chunk(*loaded);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(INTERPRET_OK, result);
    EXPECT_EQ("-1.5\n0.125\n1e+09\n", output);

    free(loaded);

    std::remove(programPath.c_str());
    std::remove(chunkPath.c_str());
}

TEST_F(ChunkFileIntegrationFixture, RejectsUnsupportedChunkFileVersion) {
    std::string chunkPath = make_temp_chunk_path();

    FILE * file = std::fopen(chunkPath.c_str(), "wb");
    ASSERT_NE(nullptr, file);
    std::fputc(0, file);
    std::fputc(255, file);
    std::fputc(0, file);
    std::fclose(file);

    testing::internal::CaptureStderr();
    chunk_t * loaded = chunk_file_load(chunkPath.c_str());
    std::string errorOutput = testing::internal::GetCapturedStderr();

    EXPECT_EQ(nullptr, loaded);
    EXPECT_NE(std::string::npos, errorOutput.find("Unsupported chunk file version"));

    std::remove(chunkPath.c_str());
}

TEST_F(ChunkFileIntegrationFixture, RejectsTruncatedChunkFile) {
    std::string chunkPath = make_temp_chunk_path();

    FILE * file = std::fopen(chunkPath.c_str(), "wb");
    ASSERT_NE(nullptr, file);
    std::fputc(0, file);
    std::fputc(0, file);
    std::fputc(1, file);
    std::fputc(0, file);
    std::fputc(0, file);
    std::fputc(0, file);
    std::fputc(2, file);
    std::fputc(0, file);
    std::fclose(file);

    testing::internal::CaptureStderr();
    chunk_t * loaded = chunk_file_load(chunkPath.c_str());
    std::string errorOutput = testing::internal::GetCapturedStderr();

    EXPECT_EQ(nullptr, loaded);
    EXPECT_TRUE(errorOutput.find("Chunk file is incomplete") != std::string::npos ||
                errorOutput.find("Unexpected file ending") != std::string::npos);

    std::remove(chunkPath.c_str());
}
