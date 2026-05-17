#include <gtest/gtest.h>
#include "../fixtures/vm_fixture.h"

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

class StdlibIntegrationFixture : public VirtualMachineFixture {
protected:
    void SetUp() override {
        VirtualMachineFixture::SetUp();
        virtual_machine_set_compile_fn(compiler_compile);
        garbage_collector_set_mark_roots_hook(compiler_mark_roots);
    }

    std::string make_temp_path() {
        char * path = clx_os_temp_make_path("cellox_stdlib_", ".clx");
        EXPECT_NE(nullptr, path);
        std::string result = path ? std::string(path) : std::string();
        std::free(path);
        return result;
    }

    bool write_file(std::string const & path, std::string const & content) {
        FILE * f = fopen(path.c_str(), "w");
        if (!f) {
            return false;
        }
        fwrite(content.c_str(), 1, content.size(), f);
        fclose(f);
        return true;
    }

    std::string run_source(char * source) {
        testing::internal::CaptureStdout();
        virtual_machine_interpret(source, /*freeProgram=*/true);
        return testing::internal::GetCapturedStdout();
    }

    int set_env(char const * name, char const * value) {
#ifdef _WIN32
        return _putenv_s(name, value);
#else
        return setenv(name, value, /*overwrite=*/1);
#endif
    }

    int unset_env(char const * name) {
#ifdef _WIN32
        return _putenv_s(name, "");
#else
        return unsetenv(name);
#endif
    }
};

TEST_F(StdlibIntegrationFixture, MathAbsViaModuleLoader) {
    std::string entryPath = make_temp_path();
    std::string entrySource = "import { abs } from \"stdlib/math.clx\";\n"
                              "printf(\"{}\\n\", abs(-4));\n"
                              "printf(\"{}\\n\", abs(6));\n";
    ASSERT_TRUE(write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = run_source(stitched);
    EXPECT_EQ("4\n6\n", output);
}

TEST_F(StdlibIntegrationFixture, StringRepeatViaModuleLoader) {
    std::string entryPath = make_temp_path();
    std::string entrySource = "import { str_repeat } from \"stdlib/string.clx\";\n"
                              "printf(\"{}\\n\", str_repeat(\"ha\", 3));\n";
    ASSERT_TRUE(write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = run_source(stitched);
    EXPECT_EQ("hahaha\n", output);
}

TEST_F(StdlibIntegrationFixture, ArraySumViaModuleLoader) {
    std::string entryPath = make_temp_path();
    std::string entrySource = "import { arr_sum } from \"stdlib/array.clx\";\n"
                              "printf(\"{}\\n\", arr_sum({10, 20, 30}));\n";
    ASSERT_TRUE(write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = run_source(stitched);
    EXPECT_EQ("60\n", output);
}

TEST_F(StdlibIntegrationFixture, IoPrintlnViaModuleLoader) {
    std::string entryPath = make_temp_path();
    std::string entrySource = "import { println } from \"stdlib/io.clx\";\n"
                              "println(\"integration\");\n";
    ASSERT_TRUE(write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = run_source(stitched);
    EXPECT_EQ("integration\n", output);
}

TEST_F(StdlibIntegrationFixture, OsNameViaModuleLoader) {
    std::string entryPath = make_temp_path();
    std::string entrySource = "import { os_name } from \"stdlib/os.clx\";\n"
                              "printf(\"{}\\n\", os_name());\n";
    ASSERT_TRUE(write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = run_source(stitched);
    EXPECT_EQ(std::string(clx_os_platform_name()) + "\n", output);
}

TEST_F(StdlibIntegrationFixture, CollectionsStackViaModuleLoader) {
    std::string entryPath = make_temp_path();
    std::string entrySource = "import { stack } from \"stdlib/collections.clx\";\n"
                              "var s = stack();\n"
                              "s.push(1).push(2);\n"
                              "printf(\"{}\\n\", s.pop());\n"
                              "printf(\"{}\\n\", s.pop());\n";
    ASSERT_TRUE(write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = run_source(stitched);
    EXPECT_EQ("2\n1\n", output);
}

TEST_F(StdlibIntegrationFixture, ViewPipelineViaModuleLoader) {
    std::string entryPath = make_temp_path();
    std::string entrySource = "import { array_view } from \"stdlib/view.clx\";\n"
                              "fun is_even(x) { return x % 2 == 0; }\n"
                              "fun square(x) { return x * x; }\n"
                              "fun sum(acc, x) { return acc + x; }\n"
                              "var v = array_view({1, 2, 3, 4}).filter(is_even).map(square);\n"
                              "printf(\"{}\\n\", v.to_array());\n"
                              "printf(\"{}\\n\", v.reduce(sum, 0));\n";
    ASSERT_TRUE(write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());
    ASSERT_NE(nullptr, stitched);

    std::string output = run_source(stitched);
    EXPECT_EQ("{4, 16}\n20\n", output);
}

TEST_F(StdlibIntegrationFixture, EnvVarOverridesBuiltinPath) {
    // Resolve the stdlib source dir relative to the test base path.
    std::string stdlibDir = TEST_PROGRAM_BASE_PATH;
    stdlibDir += "../../stdlib";

    // Set env var so the loader picks it up on the next resolution.
    set_env("CELLOX_STDLIB_DIR", stdlibDir.c_str());

    // Clear any explicit override so that the env var path is exercised.
    module_loader_set_stdlib_path(NULL);

    std::string entryPath = make_temp_path();
    std::string entrySource = "import { abs } from \"stdlib/math.clx\";\n"
                              "printf(\"{}\\n\", abs(-5));\n";
    ASSERT_TRUE(write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());

    unset_env("CELLOX_STDLIB_DIR");

    ASSERT_NE(nullptr, stitched);
    std::string output = run_source(stitched);
    EXPECT_EQ("5\n", output);
}

TEST_F(StdlibIntegrationFixture, StdlibPathOverrideOwnsMemory) {
    std::string stdlibDir = TEST_PROGRAM_BASE_PATH;
    stdlibDir += "../../stdlib";

    std::string overrideBuffer = stdlibDir;
    module_loader_set_stdlib_path(overrideBuffer.c_str());

    // Mutate the caller-owned buffer after passing it to the loader.
    // The loader must use its own copy, not this storage.
    overrideBuffer.assign("/tmp/this-path-does-not-exist");

    std::string entryPath = make_temp_path();
    std::string entrySource = "import { abs } from \"stdlib/math.clx\";\n"
                              "printf(\"{}\\n\", abs(-7));\n";
    ASSERT_TRUE(write_file(entryPath, entrySource));

    char * stitched = module_loader_load_program(entryPath.c_str());
    std::remove(entryPath.c_str());

    module_loader_set_stdlib_path(NULL);

    ASSERT_NE(nullptr, stitched);
    std::string output = run_source(stitched);
    EXPECT_EQ("7\n", output);
}
