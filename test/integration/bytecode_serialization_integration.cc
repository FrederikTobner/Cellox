#include <gtest/gtest.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

extern "C" {
#include "backend/virtual_machine.h"
#include "byte-code/chunk.h"
#include "byte-code/chunk_file.h"
#include "frontend/compilation/compiler.h"
#include "language-models/object.h"
}

static std::string derive_chunk_path(std::string path) {
    path.replace(path.size() - 3, 3, "cxcf");
    return path;
}

static std::string make_temp_base_path() {
#ifdef _WIN32
    char tempDirectory[MAX_PATH];
    DWORD directoryLength = GetTempPathA(MAX_PATH, tempDirectory);
    EXPECT_GT(directoryLength, 0u);
    EXPECT_LT(directoryLength, MAX_PATH);

    char tempFile[MAX_PATH];
    UINT result = GetTempFileNameA(tempDirectory, "clx", 0, tempFile);
    EXPECT_NE(0u, result);
    std::remove(tempFile);
    return std::string(tempFile);
#else
    char pathTemplate[] = "/tmp/cellox_bytecode_XXXXXX";
    int fd = mkstemp(pathTemplate);
    EXPECT_NE(-1, fd);
    if (fd != -1) {
        close(fd);
        std::remove(pathTemplate);
    }
    return std::string(pathTemplate);
#endif
}

static std::string make_temp_program_path() {
    return make_temp_base_path() + ".clx";
}

static std::string make_temp_chunk_path() {
    std::string path = make_temp_base_path() + ".cxcf";
    return path;
}

TEST(BytecodeSerialization, RoundTripNestedClosureExecutes) {
    const char * source =
        "fun makeAdder(a) {\n"
        "  fun inner(b) {\n"
        "    return a + b;\n"
        "  }\n"
        "\n"
        "  return inner;\n"
        "}\n"
        "\n"
        "var addTwo = makeAdder(2);\n"
        "printf(\"{}\\n\", addTwo(40));\n";

    std::string programPath = make_temp_program_path();
    std::string chunkPath = derive_chunk_path(programPath);

    virtual_machine_init();
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
    virtual_machine_free();

    std::remove(programPath.c_str());
    std::remove(chunkPath.c_str());
}

TEST(BytecodeSerialization, RoundTripPreservesCodeAndLineInfo) {
    const char * source =
        "var x = 1;\n"
        "x += 41;\n"
        "printf(\"{}\\n\", x);\n";

    std::string programPath = make_temp_program_path();
    std::string chunkPath = derive_chunk_path(programPath);

    virtual_machine_init();
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
    virtual_machine_free();

    std::remove(programPath.c_str());
    std::remove(chunkPath.c_str());
}

TEST(BytecodeSerialization, RoundTripPreservesNegativeAndSpecialNumbers) {
    const char * source =
        "printf(\"{}\\n\", -1.5);\n"
        "printf(\"{}\\n\", 0.125);\n"
        "printf(\"{}\\n\", 1000000000.25);\n";

    std::string programPath = make_temp_program_path();
    std::string chunkPath = derive_chunk_path(programPath);

    virtual_machine_init();
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
    virtual_machine_free();

    std::remove(programPath.c_str());
    std::remove(chunkPath.c_str());
}

TEST(BytecodeSerialization, RejectsUnsupportedChunkVersion) {
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

TEST(BytecodeSerialization, RejectsTruncatedChunkFile) {
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
