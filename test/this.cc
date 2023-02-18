#include "test_cellox.hh"

#include <gtest/gtest.h>

TEST(This, InMethod) {
    test_cellox_program("this/this_in_method.clx", "baz\n");
}

TEST(This, InSuperClass) {
    test_cellox_program("this/this_in_superclass.clx", "a\nb\n");
}

TEST(This, OutsideClass) {
    test_failing_cellox_program("this/this_outside_class.clx", "[line 1] Error at 'this': Can't use 'this' outside of a class.\n");
}