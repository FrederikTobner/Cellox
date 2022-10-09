#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(EscapeSequences, alphabet)
{
    test_cellox_program("escapeSequences/alpha.clx", "\a\b\f\n\t\v\r\n", false);
}

TEST(EscapeSequences, specialCharacters)
{
    test_cellox_program("escapeSequences/special_characters.clx", "\"\'\\\?\n", false);
}

TEST(EscapeSequences, octalNumbers)
{
    test_cellox_program("escapeSequences/octal_number.clx", "\1\55\147\n", false);
}

TEST(EscapeSequences, hexadecimalNumber)
{
    test_cellox_program("escapeSequences/hex_number.clx", "\x1\xaa\x91\n", false);
}