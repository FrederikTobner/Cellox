#include <gtest/gtest.h>
#include "test_cellox.hh"

extern "C"
{
#include "init.h"
}

TEST(Comments, SingleLine)
{
    test_cellox_program("comments/singleLine.clx", "ok\n");
}

TEST(Comments, MultiLine)
{
    test_cellox_program("comments/multiLine.clx", "ok\n");
}