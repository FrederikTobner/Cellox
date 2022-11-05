#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Fields, Call)
{
    test_failing_cellox_program("fields/call.clx", "Can only call functions and classes.\n[line 9] in script\n");
}

TEST(Fields, GetOnBool)
{
    test_failing_cellox_program("fields/get_on_bool.clx", "Only instances have properties.\n[line 1] in script\n");
}

TEST(Fields, GetOnClass)
{
    test_failing_cellox_program("fields/get_on_class.clx", "Only instances have properties.\n[line 2] in script\n");
}

TEST(Fields, GetOnFunction)
{
    test_failing_cellox_program("fields/get_on_function.clx", "Only instances have properties.\n[line 3] in script\n");
}

TEST(Fields, GetOnNull)
{
    test_failing_cellox_program("fields/get_on_null.clx", "Only instances have properties.\n[line 1] in script\n");
}

TEST(Fields, GetOnNumber)
{
    test_failing_cellox_program("fields/get_on_number.clx", "Only instances have properties.\n[line 1] in script\n");
}

TEST(Fields, GetOnString)
{
    test_failing_cellox_program("fields/get_on_string.clx", "Only instances have properties.\n[line 1] in script\n");
}

TEST(Fields, Inherit)
{
    test_cellox_program("fields/inherit.clx", "test\n");
}

TEST(Fields, OnInstance)
{
    test_cellox_program("fields/on_instance.clx", "bar!\nbaz!\nbar!\nbaz!\n");
}

TEST(Fields, SetOnBool)
{
    test_failing_cellox_program("fields/set_on_bool.clx", "Only instances have fields.\n[line 1] in script\n");
}

TEST(Fields, SetOnClass)
{
    test_failing_cellox_program("fields/set_on_class.clx", "Only instances have fields.\n[line 2] in script\n");
}

TEST(Fields, SetOnFunction)
{
    test_failing_cellox_program("fields/set_on_function.clx", "Only instances have fields.\n[line 3] in script\n");
}

TEST(Fields, SetOnNull)
{
    test_failing_cellox_program("fields/set_on_null.clx", "Only instances have fields.\n[line 1] in script\n");
}

TEST(Fields, SetOnNumber)
{
    test_failing_cellox_program("fields/set_on_number.clx", "Only instances have fields.\n[line 1] in script\n");
}

TEST(Fields, SetOnString)
{
    test_failing_cellox_program("fields/set_on_string.clx", "Only instances have fields.\n[line 1] in script\n");
}

TEST(Fields, SerializeProperties)
{
    test_cellox_program("fields/serialize.clx", "{value: 10, text: \"test\"}\n");
}

TEST(Fields, UndefienedProperty)
{
    test_failing_cellox_program("fields/undefiened.clx", "Undefined property 'test'.\n[line 4] in script\n");
}