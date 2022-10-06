#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Comments, SingleLine)
{
    test_cellox_program("comments/singleLine.clx", "ok\n", false);
}

TEST(Comments, MultiLine)
{
    test_cellox_program("comments/multiLine.clx", "ok\n", false);
}