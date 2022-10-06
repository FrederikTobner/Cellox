#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Classes, Constructor)
{
    test_cellox_program("class/constructor.clx", "a field\n", false);
}

TEST(Classes, EmptyClass)
{
    test_cellox_program("class/emptyClass.clx", "Foo\n", false);
}

TEST(Classes, InheritField)
{
    test_cellox_program("class/inheritField.clx", "test\n", false);
}

TEST(Classes, InheritMethod)
{
    test_cellox_program("class/inheritMethod.clx", "hello\n", false);
}

TEST(Classes, InheritSelf)
{
    test_cellox_program("class/inheritSelf.clx", "[line 1] Error at 'Foo': A class can't inherit from itself.\n", true);
}

TEST(Classes, PrintInstance)
{
    test_cellox_program("class/printInstance.clx", "Foo instance\n", false);
}