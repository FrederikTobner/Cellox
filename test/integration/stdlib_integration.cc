#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

extern "C" {
#include "backend/garbage_collector.h"
#include "backend/virtual_machine.h"
#include "clx_os/platform.h"
#include "clx_os/temp.h"
#include "frontend/compiler.h"
#include "module-loading/module_loader.h"
}

static std::string stdlib_make_temp_path() {
    char * path = clx_os_temp_make_path("cellox_stdlib_", ".clx");
    EXPECT_NE(nullptr, path);
    std::string result = path ? std::string(path) : std::string();
    std::free(path);
    return result;
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

TEST(StdlibIntegration, MathAbsViaModuleLoader) {
    std::string entryPath = stdlib_make_temp_path();
    std::string entrySource =
        "import { abs } from \"stdlib/math.clx\";\n"
        "printf(\"{}\\n\", abs(-4));\n"
        "printf(\"{}\\n\", abs(6));\n";
    ASSERT_TRUE(stdlib_write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = stdlib_run_source(stitched);
    EXPECT_EQ("4\n6\n", output);
}

TEST(StdlibIntegration, StringRepeatViaModuleLoader) {
    std::string entryPath = stdlib_make_temp_path();
    std::string entrySource =
        "import { str_repeat } from \"stdlib/string.clx\";\n"
        "printf(\"{}\\n\", str_repeat(\"ha\", 3));\n";
    ASSERT_TRUE(stdlib_write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = stdlib_run_source(stitched);
    EXPECT_EQ("hahaha\n", output);
}

TEST(StdlibIntegration, ArraySumViaModuleLoader) {
    std::string entryPath = stdlib_make_temp_path();
    std::string entrySource =
        "import { arr_sum } from \"stdlib/array.clx\";\n"
        "printf(\"{}\\n\", arr_sum({10, 20, 30}));\n";
    ASSERT_TRUE(stdlib_write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = stdlib_run_source(stitched);
    EXPECT_EQ("60\n", output);
}

TEST(StdlibIntegration, IoPrintlnViaModuleLoader) {
    std::string entryPath = stdlib_make_temp_path();
    std::string entrySource =
        "import { println } from \"stdlib/io.clx\";\n"
        "println(\"integration\");\n";
    ASSERT_TRUE(stdlib_write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = stdlib_run_source(stitched);
    EXPECT_EQ("integration\n", output);
}

TEST(StdlibIntegration, OsNameViaModuleLoader) {
    std::string entryPath = stdlib_make_temp_path();
    std::string entrySource =
        "import { os_name } from \"stdlib/os.clx\";\n"
        "printf(\"{}\\n\", os_name());\n";
    ASSERT_TRUE(stdlib_write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = stdlib_run_source(stitched);
    EXPECT_EQ(std::string(clx_os_platform_name()) + "\n", output);
}

TEST(StdlibIntegration, CollectionsStackViaModuleLoader) {
    std::string entryPath = stdlib_make_temp_path();
    std::string entrySource =
        "import { stack } from \"stdlib/collections.clx\";\n"
        "var s = stack();\n"
        "s.push(1).push(2);\n"
        "printf(\"{}\\n\", s.pop());\n"
        "printf(\"{}\\n\", s.pop());\n";
    ASSERT_TRUE(stdlib_write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = stdlib_run_source(stitched);
    EXPECT_EQ("2\n1\n", output);
}

TEST(StdlibIntegration, ViewPipelineViaModuleLoader) {
    std::string entryPath = stdlib_make_temp_path();
    std::string entrySource =
        "import { array_view } from \"stdlib/view.clx\";\n"
        "fun is_even(x) { return x % 2 == 0; }\n"
        "fun square(x) { return x * x; }\n"
        "fun sum(acc, x) { return acc + x; }\n"
        "var v = array_view({1, 2, 3, 4}).filter(is_even).map(square);\n"
        "printf(\"{}\\n\", v.to_array());\n"
        "printf(\"{}\\n\", v.reduce(sum, 0));\n";
    ASSERT_TRUE(stdlib_write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = stdlib_run_source(stitched);
    EXPECT_EQ("{4, 16}\n20\n", output);
}

TEST(StdlibIntegration, StdlibPathOverrideOwnsMemory) {
    std::string stdlibDir = TEST_PROGRAM_BASE_PATH;
    stdlibDir += "../stdlib";

    std::string overrideBuffer = stdlibDir;
    module_loader_set_stdlib_path(overrideBuffer.c_str());

    // Mutate the caller-owned buffer after passing it to the loader.
    // The loader must use its own copy, not this storage.
    overrideBuffer.assign("/tmp/this-path-does-not-exist");

    std::string entryPath = stdlib_make_temp_path();
    std::string entrySource =
        "import { abs } from \"stdlib/math.clx\";\n"
        "printf(\"{}\\n\", abs(-7));\n";
    ASSERT_TRUE(stdlib_write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());

    module_loader_set_stdlib_path(NULL);

    ASSERT_NE(nullptr, stitched);
    std::string output = stdlib_run_source(stitched);
    EXPECT_EQ("7\n", output);
}
