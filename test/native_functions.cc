#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(NativeFunctions, ArrayLength)
{
    test_cellox_program("native_functions/array_length.clx", "5\n");
}

TEST(NativeFunctions, AsciToNumerical)
{
    test_cellox_program("native_functions/asci_to_numerical.clx", "70");
}

TEST(NativeFunctions, ClassOf)
{
    test_cellox_program("native_functions/class_of.clx", "Foo\ntrue\n");
}

TEST(NativeFunctions, NumericalToAsci)
{
    test_cellox_program("native_functions/numerical_to_asci.clx", "F");
}

TEST(NativeFunctions, StringLength)
{
    test_cellox_program("native_functions/string_length.clx", "0\n6\n11\n");
}