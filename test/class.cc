#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Classes, Constructor)
{
    test_cellox_program("class/constructor.clx", "a field\n");
}

TEST(Classes, EmptyClass)
{
    test_cellox_program("class/empty_class.clx", "Foo\n");
}

TEST(Classes, InheritMethod)
{
    test_cellox_program("class/inherit_method.clx", "hello\n");
}

TEST(Classes, InheritSelf)
{
    test_failing_cellox_program("class/inherit_self.clx", "[line 1] Error at 'Foo': A class can't inherit from itself.\n");
}

TEST(Classes, PrintInstance)
{
    test_cellox_program("class/print_instance.clx", "{}\n");
}