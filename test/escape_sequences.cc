#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(EscapeSequences, alphabet)
{
    test_cellox_program("escapeSequences/alpha.clx", "\a\b\f\n\t\v\r\n");
}

TEST(EscapeSequences, specialCharacters)
{
    test_cellox_program("escapeSequences/special_characters.clx", "\"\'\\\?\n");
}

TEST(EscapeSequences, octalNumbers)
{
    test_cellox_program("escapeSequences/octal_number.clx", "\1\55\147\n");
}

TEST(EscapeSequences, hexadecimalNumber)
{
    test_cellox_program("escapeSequences/hex_number.clx", "\x1\xaa\x91\n");
}