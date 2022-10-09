#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(NativeFunctions, strlen)
{
    test_cellox_program("nativeFunctions/string_length.clx", "0\n6\n11\n", false);
}