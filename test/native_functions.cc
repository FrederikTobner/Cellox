#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(NativeFunctions, class_of)
{
    test_cellox_program("nativeFunctions/class_of.clx", "Foo\ntrue\n");
}

TEST(NativeFunctions, on_linux)
{
#ifdef _WIN32
        test_cellox_program("nativeFunctions/on_linux.clx", "false\n");
#elif linux
        test_cellox_program("nativeFunctions/on_linux.clx", "true\n");
#endif
    
}

TEST(NativeFunctions, on_windows)
{
    
#ifdef _WIN32
        test_cellox_program("nativeFunctions/on_windows.clx", "true\n");
#elif linux
        test_cellox_program("nativeFunctions/on_windows.clx", "false\n");
#endif
}

TEST(NativeFunctions, String_Length)
{
    test_cellox_program("nativeFunctions/string_length.clx", "0\n6\n11\n");
}