#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Comments, SingleLine)
{
    test_program("comments/singleLine.clx", "ok\n");
}

TEST(Comments, MultiLine)
{
    test_program("comments/multiLine.clx", "ok\n");
}