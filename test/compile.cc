#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Compile, Simple) {
    test_compiled_cellox_program("compile/simple.clx", "Hello World!\n");
}