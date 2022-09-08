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