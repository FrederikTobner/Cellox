#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(NativeFunctions, strlen)
{
    test_program("nativeFunctions/stringLength.clx", "0\n6\n11\n");
}