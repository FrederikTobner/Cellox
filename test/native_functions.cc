#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(NativeFunctions, class_of)
{
    test_cellox_program("nativeFunctions/class_of.clx", "Foo\ntrue\n", false);
}

TEST(NativeFunctions, on_linux)
{
#ifdef _WIN32
        test_cellox_program("nativeFunctions/on_linux.clx", "false\n", false);
#endif
#ifdef linux
        test_cellox_program("nativeFunctions/on_linux.clx", "true\n", true);
#endif
    
}

TEST(NativeFunctions, on_windows)
{
    
#ifdef _WIN32
        test_cellox_program("nativeFunctions/on_windows.clx", "true\n", false);
#endif
#ifdef linux
        test_cellox_program("nativeFunctions/on_windows.clx", "false\n", true);
#endif
}

TEST(NativeFunctions, String_Length)
{
    test_cellox_program("nativeFunctions/string_length.clx", "0\n6\n11\n", false);
}