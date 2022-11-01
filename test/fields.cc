#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Fields, Call)
{
    test_failing_cellox_program("fields/call.clx", "Can only call functions and classes.\n[line 9] in script\n");
}

TEST(Fields, Inherit)
{
    test_cellox_program("fields/inherit.clx", "test\n");
}

TEST(Fields, SerializeProperties)
{
    test_cellox_program("fields/serialize.clx", "{value: 10, text: \"test\"}\n");
}

TEST(Fields, UndefienedProperty)
{
    test_failing_cellox_program("fields/undefiened.clx", "Undefined property 'test'.\n[line 4] in script\n");
}