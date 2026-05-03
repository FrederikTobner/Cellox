#include <gtest/gtest.h>

#include <cstdlib>
#include <cstdio>
#include <random>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

extern "C" {
#include "backend/virtual_machine.h"
#include "byte-code/chunk_file.h"
#include "frontend/compiler.h"
}

static std::string fuzz_derive_chunk_path(std::string path) {
    path.replace(path.size() - 3, 3, "cxcf");
    return path;
}

static std::string fuzz_make_temp_base_path() {
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
    char pathTemplate[] = "/tmp/cellox_fuzz_bytecode_XXXXXX";
    int fd = mkstemp(pathTemplate);
    EXPECT_NE(-1, fd);
    if (fd != -1) {
        close(fd);
        std::remove(pathTemplate);
    }
    return std::string(pathTemplate);
#endif
}

static std::string fuzz_make_temp_program_path() {
    return fuzz_make_temp_base_path() + ".clx";
}

TEST(PropertyFuzzIntegration, RandomNestedClosureRoundTrip) {
    std::mt19937 rng(987654u);
    std::uniform_int_distribution<int> valueDist(-500, 500);

    for (int i = 0; i < 30; i++) {
        int a = valueDist(rng);
        int b = valueDist(rng);
        int expected = a + b;

        std::ostringstream source;
         source << "fun outer() {\n"
             << "  var x = " << a << ";\n"
             << "  fun inner() { return x + " << b << "; }\n"
             << "  printf(\"{}\\n\", inner());\n"
             << "}\n"
             << "outer();\n";

        std::string programPath = fuzz_make_temp_program_path();
        std::string chunkPath = fuzz_derive_chunk_path(programPath);

        virtual_machine_init();
        object_function_t * function = compiler_compile(source.str().c_str());
        ASSERT_NE(nullptr, function);
        ASSERT_EQ(0,
                  chunk_file_store(function->chunk, programPath.c_str(), static_cast<chunk_file_compile_flag>(0)));

        chunk_t * loaded = chunk_file_load(chunkPath.c_str());
        ASSERT_NE(nullptr, loaded);

        testing::internal::CaptureStdout();
        interpret_result result = virtual_machine_run_chunk(*loaded);
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(INTERPRET_OK, result);
        EXPECT_EQ(std::to_string(expected) + "\n", output);

        free(loaded);
        virtual_machine_free();

        std::remove(programPath.c_str());
        std::remove(chunkPath.c_str());
    }
}
