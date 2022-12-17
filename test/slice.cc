#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Slice, Array)
{
    test_cellox_program("slice/array.clx", "{2, 3}\n");
}

TEST(Slice, String)
{
    test_cellox_program("slice/string.clx", "Hello\n");
}
