#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Classes, EmptyClass)
{
    test_program("class/emptyClass.clx", "Foo\n");
}