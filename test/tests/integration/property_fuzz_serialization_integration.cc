#include "../fixtures/vm_fixture.h"
#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <random>
#include <sstream>
#include <string>

extern "C" {
#include "backend/virtual_machine.h"
#include "byte-code/chunk_file.h"
#include "clx_os/temp.h"
#include "frontend/compiler.h"
}

class PropertyFuzzSerializationIntegrationFixture : public VirtualMachineFixture {
  protected:
    std::string derive_chunk_path(std::string path) {
        path.replace(path.size() - 3, 3, "cxcf");
        return path;
    }

    std::string make_temp_base_path() {
        char * path = clx_os_temp_make_path("cellox_fuzz_bytecode_", "");
        EXPECT_NE(nullptr, path);
        std::string result = path ? std::string(path) : std::string();
        std::free(path);
        return result;
    }

    std::string make_temp_program_path() {
        return make_temp_base_path() + ".clx";
    }

    void reset_vm() {
        reset();
    }
};

TEST_F(PropertyFuzzSerializationIntegrationFixture, RandomNestedClosureRoundTrip) {
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

        std::string programPath = make_temp_program_path();
        std::string chunkPath = derive_chunk_path(programPath);

        if (i > 0) {
            reset_vm();
        }

        object_function_t * function = compiler_compile(source.str().c_str());
        ASSERT_NE(nullptr, function);
        ASSERT_EQ(0, chunk_file_store(function->chunk, programPath.c_str(), static_cast<chunk_file_compile_flag>(0)));

        chunk_t * loaded = chunk_file_load(chunkPath.c_str());
        ASSERT_NE(nullptr, loaded);

        testing::internal::CaptureStdout();
        interpret_result result = virtual_machine_run_chunk(*loaded);
        std::string output = testing::internal::GetCapturedStdout();

        EXPECT_EQ(INTERPRET_OK, result);
        EXPECT_EQ(std::to_string(expected) + "\n", output);

        free(loaded);

        if (i < 29) {
            reset_vm();
        }

        std::remove(programPath.c_str());
        std::remove(chunkPath.c_str());
    }
}
