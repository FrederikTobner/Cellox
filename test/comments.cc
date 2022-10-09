#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Comments, SingleLine)
{
    test_cellox_program("comments/single_line.clx", "ok\n", false);
}

TEST(Comments, MultiLine)
{
    test_cellox_program("comments/multi_line.clx", "ok\n", false);
}