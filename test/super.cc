#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Super, incompleteSuper)
{
    test_cellox_program("super/incomplete_super.clx", "[line 10] Error at ';': Expect superclass method name.\n", true);
}

TEST(Super, outsideClass)
{
    test_cellox_program("super/outside_class.clx", "[line 1] Error at 'super': Can't use 'super' outside of a class.\n", true);
}

TEST(Super, withoutDot)
{
    test_cellox_program("super/without_dot.clx", "[line 10] Error at ';': Expect '.' after 'super'.\n", true);
}