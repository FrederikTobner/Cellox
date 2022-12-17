#include <gtest/gtest.h>
#include "test_cellox.hh"

TEST(LiteralExpressions, BinaryNumbers)
{
    test_cellox_program("literal_expressions/binary_numbers.clx", "15\n256\n");
}

TEST(LiteralExpressions, Booleans)
{
    test_cellox_program("literal_expressions/booleans.clx", "true\nfalse\n");
}

TEST(LiteralExpressions, HexNumbers)
{
    test_cellox_program("literal_expressions/hex_numbers.clx", "175\n175\n17\n");
}

TEST(LiteralExpressions, Null)
{
    test_cellox_program("literal_expressions/null.clx", "null\n");
}

TEST(LiteralExpressions, Numbers)
{
    test_cellox_program("literal_expressions/numbers.clx", "123\n-10\n123.456\n-0.001\n");
}

TEST(LiteralExpressions, Strings)
{
    #ifdef _WIN32
        test_cellox_program("literal_expressions/strings.clx", "\na\nA\n123\n!\"$%&/()=?\n1\r\n2\r\n3\n");
    #elif linux
        test_cellox_program("literal_expressions/strings.clx", "\na\nA\n123\n!\"$%&/()=?\n1\n2\n3\n"); // TODO: Is that even correct under linux?
    #endif 
}
