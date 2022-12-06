#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(NativeFunctions, class_of)
{
    test_cellox_program("nativeFunctions/class_of.clx", "Foo\ntrue\n");
}

TEST(NativeFunctions, String_Length)
{
    test_cellox_program("nativeFunctions/string_length.clx", "0\n6\n11\n");
}