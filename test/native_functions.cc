#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(NativeFunctions, ArrayLength)
{
    test_cellox_program("native_functions/array_length.clx", "5\n");
}

TEST(NativeFunctions, ClassOf)
{
    test_cellox_program("native_functions/class_of.clx", "Foo\ntrue\n");
}

TEST(NativeFunctions, StringLength)
{
    test_cellox_program("native_functions/string_length.clx", "0\n6\n11\n");
}