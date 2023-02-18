#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Classes, Constructor) {
    test_cellox_program("class/constructor.clx", "a field\n");
}

TEST(Classes, EmptyClass) {
    test_cellox_program("class/empty_class.clx", "Foo\n");
}

TEST(Classes, InheritFromBoolean) {
    test_failing_cellox_program("class/inherit_from_bool.clx", "Superclass must be a class but is a boolean value\n[line 2] in script\n");
}

TEST(Classes, InheritFromFunction) {
    test_failing_cellox_program("class/inherit_from_function.clx", "Superclass must be a class but is a closure object\n[line 2] in script\n");
}

TEST(Classes, InheritFromInstance) {
    test_failing_cellox_program("class/inherit_from_instance.clx", "Superclass must be a class but is a Foo object\n[line 3] in script\n");
}

TEST(Classes, InheritMethod) {
    test_cellox_program("class/inherit_method.clx", "hello\n");
}

TEST(Classes, InheritFromNull) {
    test_failing_cellox_program("class/inherit_from_null.clx", "Superclass must be a class but is a undefiened value\n[line 2] in script\n");
}

TEST(Classes, InheritFromNumber) {
    test_failing_cellox_program("class/inherit_from_number.clx", "Superclass must be a class but is a numerical value\n[line 2] in script\n");
}

TEST(Classes, InheritSelf) {
    test_failing_cellox_program("class/inherit_self.clx", "[line 1] Error at 'Foo': A class can't inherit from itself.\n");
}

TEST(Classes, InitMissingArgs) {
    test_failing_cellox_program("class/init_missing_args.clx", "Expected 2 arguments but got 1.\n[line 5] in script\n");
}

TEST(Classes, NoExplizitInit) {
    test_cellox_program("class/no_explizit_init.clx", "{}\n");
}

TEST(Classes, PrintInstance) {
    test_cellox_program("class/print_instance.clx", "{}\n");
}