#include <gtest/gtest.h>

#include "test_cellox.hh"

extern "C"
{
#include "init.h"
}

TEST(IndexOperator, String)
{
    test_cellox_program("indexOperator/string.clx", "t\ne\ns\nt\n");
}