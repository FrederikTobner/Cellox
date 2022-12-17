#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(EscapeSequences, alphabet)
{
    test_cellox_program("escape_sequences/alpha.clx", "\a\b\f\n\t\v\r\n");
}

TEST(EscapeSequences, hexadecimalNumber)
{
    test_cellox_program("escape_sequences/hex_number.clx", "\x1\xaa\x91\n");
}

TEST(EscapeSequences, octalNumbers)
{
    test_cellox_program("escape_sequences/octal_number.clx", "\1\55\147\n");
}

TEST(EscapeSequences, specialCharacters)
{
    test_cellox_program("escape_sequences/special_characters.clx", "\"\'\\\?\n");
}

TEST(EscapeSequences, unknown)
{
    test_failing_cellox_program("escape_sequences/unknown.clx", "[line 1] Error at '\"\\p\"': Unknown escape sequence in string\n");
}