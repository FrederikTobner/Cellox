#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Classes, Constructor)
{
    test_cellox_program("class/constructor.clx", "a field\n", false);
}

TEST(Classes, EmptyClass)
{
    test_cellox_program("class/empty_class.clx", "Foo\n", false);
}

TEST(Classes, InheritField)
{
    test_cellox_program("class/inherit_field.clx", "test\n", false);
}

TEST(Classes, InheritMethod)
{
    test_cellox_program("class/inherit_method.clx", "hello\n", false);
}

TEST(Classes, InheritSelf)
{
    test_cellox_program("class/inherit_self.clx", "[line 1] Error at 'Foo': A class can't inherit from itself.\n", true);
}

TEST(Classes, PrintInstance)
{
    test_cellox_program("class/print_instance.clx", "{}\n", false);
}

TEST(Classes, SerializeFields)
{
    test_cellox_program("class/serialize_fields.clx", "{value: 10, text: \"test\"}\n", false);
}

TEST(Classes, UndefienedProperty)
{
    test_cellox_program("class/undefiened_property.clx", "Undefined property 'test'.\n[line 4] in script\n", true);
}