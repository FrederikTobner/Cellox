#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(EscapeSequences, alphabet)
{
    test_program("escapeSequences/alpha.clx", "\a\b\f\n\t\v\r\n");
}

TEST(EscapeSequences, specialCharacters)
{
    test_program("escapeSequences/specialcharacters.clx", "\"\'\\\?\n");
}

TEST(EscapeSequences, octalNumbers)
{
    test_program("escapeSequences/octalNumber.clx", "\1\55\147\n");
}

TEST(EscapeSequences, hexadecimalNumber)
{
    test_program("escapeSequences/hexNumber.clx", "\x1\xaa\x91\n");
}