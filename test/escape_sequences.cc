#include <gtest/gtest.h>

#include "test_cellox.hh"

extern "C"
{
#include "init.h"
}

TEST(EscapeSequences, alphabet)
{
    test_cellox_program("escapeSequences/alpha.clx", "\a\b\f\n\t\v\r\n");
}

TEST(EscapeSequences, specialCharacters)
{
    test_cellox_program("escapeSequences/specialcharacters.clx", "\"\'\\\?\n");
}

TEST(EscapeSequences, octalNumbers)
{
    test_cellox_program("escapeSequences/octalNumber.clx", "\1\55\147\n");
}

TEST(EscapeSequences, hexadecimalNumber)
{
    test_cellox_program("escapeSequences/hexNumber.clx", "\x1\xaa\x91\n");
}