#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(IndexOperator, String)
{
    test_cellox_program("indexOperator/string.clx", "t\ne\ns\nt\n", false);
}