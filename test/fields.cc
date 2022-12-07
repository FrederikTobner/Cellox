#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Fields, Call)
{
    test_failing_cellox_program("fields/call.clx", "Can only call functions and classes, but call expression was performed with a numerical value\n[line 7] in script\n");
}

TEST(Fields, GetOnBool)
{
    test_failing_cellox_program("fields/get_on_bool.clx", "Only instances have properties but get expression but a boolean value was used\n[line 1] in script\n");
}

TEST(Fields, GetOnClass)
{
    test_failing_cellox_program("fields/get_on_class.clx", "Only instances have properties but get expression but a class object was used\n[line 4] in script\n");
}

TEST(Fields, GetOnFunction)
{
    test_failing_cellox_program("fields/get_on_function.clx", "Only instances have properties but get expression but a closure object was used\n[line 5] in script\n");
}

TEST(Fields, GetOnNull)
{
    test_failing_cellox_program("fields/get_on_null.clx", "Only instances have properties but get expression but a undefiened value was used\n[line 1] in script\n");
}

TEST(Fields, GetOnNumber)
{
    test_failing_cellox_program("fields/get_on_number.clx", "Only instances have properties but get expression but a numerical value was used\n[line 1] in script\n");
}

TEST(Fields, GetOnString)
{
    test_failing_cellox_program("fields/get_on_string.clx", "Only instances have properties but get expression but a string object was used\n[line 1] in script\n");
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
    test_failing_cellox_program("fields/set_on_bool.clx", "Only instances have fields but was called with a boolean value\n[line 1] in script\n");
}

TEST(Fields, SetOnClass)
{
    test_failing_cellox_program("fields/set_on_class.clx", "Only instances have fields but was called with a class object\n[line 2] in script\n");
}

TEST(Fields, SetOnFunction)
{
    test_failing_cellox_program("fields/set_on_function.clx", "Only instances have fields but was called with a closure object\n[line 5] in script\n");
}

TEST(Fields, SetOnNull)
{
    test_failing_cellox_program("fields/set_on_null.clx", "Only instances have fields but was called with a undefiened value\n[line 1] in script\n");
}

TEST(Fields, SetOnNumber)
{
    test_failing_cellox_program("fields/set_on_number.clx", "Only instances have fields but was called with a numerical value\n[line 1] in script\n");
}

TEST(Fields, SetOnString)
{
    test_failing_cellox_program("fields/set_on_string.clx", "Only instances have fields but was called with a string object\n[line 1] in script\n");
}

TEST(Fields, SerializeProperties)
{
    test_cellox_program("fields/serialize.clx", "{value: 10, text: \"test\"}\n");
}

TEST(Fields, UndefienedProperty)
{
    test_failing_cellox_program("fields/undefiened.clx", "Undefined property 'test'.\n[line 6] in script\n");
}