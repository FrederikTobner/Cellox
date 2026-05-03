#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

extern "C" {
#include "backend/garbage_collector.h"
#include "backend/virtual_machine.h"
#include "frontend/compilation/compiler.h"
#include "module_loader.h"
}

static std::string stdlib_make_temp_path() {
#ifdef _WIN32
    char tempDirectory[MAX_PATH];
    GetTempPathA(MAX_PATH, tempDirectory);
    char tempFile[MAX_PATH];
    GetTempFileNameA(tempDirectory, "clx", 0, tempFile);
    std::remove(tempFile);
    return std::string(tempFile) + ".clx";
#else
    char pathTemplate[] = "/tmp/cellox_stdlib_XXXXXX";
    int fd = mkstemp(pathTemplate);
    if (fd != -1) {
        close(fd);
        std::remove(pathTemplate);
    }
    return std::string(pathTemplate) + ".clx";
#endif
}

/// Writes text to a file. Returns true on success.
static bool stdlib_write_file(std::string const & path, std::string const & content) {
    FILE * f = fopen(path.c_str(), "w");
    if (!f) {
        return false;
    }
    fwrite(content.c_str(), 1, content.size(), f);
    fclose(f);
    return true;
}

/// Runs a cellox source (already stitched) and captures stdout.
static std::string stdlib_run_source(char * source) {
    virtual_machine_set_compile_fn(compiler_compile);
    garbage_collector_set_mark_roots_hook(compiler_mark_roots);
    virtual_machine_init();
    testing::internal::CaptureStdout();
    virtual_machine_interpret(source, /*freeProgram=*/true);
    std::string output = testing::internal::GetCapturedStdout();
    virtual_machine_free();
    return output;
}

// ── math.clx: abs ────────────────────────────────────────────────────────────

TEST(StdlibIntegration, MathAbsViaModuleLoader) {
    std::string stdlibPath = TEST_PROGRAM_BASE_PATH;
    stdlibPath += "../stdlib/math.clx";

    std::string entryPath = stdlib_make_temp_path();
    std::string entrySource =
        "import { abs } from \"" + stdlibPath + "\";\n"
        "printf(\"{}\\n\", abs(-4));\n"
        "printf(\"{}\\n\", abs(6));\n";
    ASSERT_TRUE(stdlib_write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = stdlib_run_source(stitched);
    EXPECT_EQ("4\n6\n", output);
}

// ── string.clx: str_repeat ───────────────────────────────────────────────────

TEST(StdlibIntegration, StringRepeatViaModuleLoader) {
    std::string stdlibPath = TEST_PROGRAM_BASE_PATH;
    stdlibPath += "../stdlib/string.clx";

    std::string entryPath = stdlib_make_temp_path();
    std::string entrySource =
        "import { str_repeat } from \"" + stdlibPath + "\";\n"
        "printf(\"{}\\n\", str_repeat(\"ha\", 3));\n";
    ASSERT_TRUE(stdlib_write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = stdlib_run_source(stitched);
    EXPECT_EQ("hahaha\n", output);
}

// ── array.clx: arr_sum ───────────────────────────────────────────────────────

TEST(StdlibIntegration, ArraySumViaModuleLoader) {
    std::string stdlibPath = TEST_PROGRAM_BASE_PATH;
    stdlibPath += "../stdlib/array.clx";

    std::string entryPath = stdlib_make_temp_path();
    std::string entrySource =
        "import { arr_sum } from \"" + stdlibPath + "\";\n"
        "printf(\"{}\\n\", arr_sum({10, 20, 30}));\n";
    ASSERT_TRUE(stdlib_write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = stdlib_run_source(stitched);
    EXPECT_EQ("60\n", output);
}

// ── io.clx: println ──────────────────────────────────────────────────────────

TEST(StdlibIntegration, IoPrintlnViaModuleLoader) {
    std::string stdlibPath = TEST_PROGRAM_BASE_PATH;
    stdlibPath += "../stdlib/io.clx";

    std::string entryPath = stdlib_make_temp_path();
    std::string entrySource =
        "import { println } from \"" + stdlibPath + "\";\n"
        "println(\"integration\");\n";
    ASSERT_TRUE(stdlib_write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = stdlib_run_source(stitched);
    EXPECT_EQ("integration\n", output);
}

// ── os.clx: os_name ──────────────────────────────────────────────────────────

TEST(StdlibIntegration, OsNameViaModuleLoader) {
    std::string stdlibPath = TEST_PROGRAM_BASE_PATH;
    stdlibPath += "../stdlib/os.clx";

    std::string entryPath = stdlib_make_temp_path();
    std::string entrySource =
        "import { os_name } from \"" + stdlibPath + "\";\n"
        "printf(\"{}\\n\", os_name());\n";
    ASSERT_TRUE(stdlib_write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = stdlib_run_source(stitched);
#ifdef __linux__
    EXPECT_EQ("linux\n", output);
#elif defined(__APPLE__)
    EXPECT_EQ("macos\n", output);
#elif defined(_WIN32)
    EXPECT_EQ("windows\n", output);
#endif
}
