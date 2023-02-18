#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Super, inClosureInInheritedMethod) {
    test_cellox_program("super/in_closure_in_inherited_method.clx", "A\n");
}

TEST(Super, inInheritedMethod) {
    test_cellox_program("super/in_inherited_method.clx", "A\n");
}

TEST(Super, incompleteSuper) {
    test_failing_cellox_program("super/incomplete_super.clx", "[line 7] Error at ';': Expect superclass method name.\n");
}

TEST(Super, methodDoesNotExist) {
    test_failing_cellox_program("super/method_does_not_exist.clx", "Undefined property 'doesNotExist'.\n[line 5] in foo()\n[line 9] in script\n");
}

TEST(Super, noSupperClass) {
    test_failing_cellox_program("super/no_superclass.clx", "[line 3] Error at 'super': Can't use 'super' in a class with no superclass.\n");
}

TEST(Super, outsideClass) {
    test_failing_cellox_program("super/outside_class.clx", "[line 1] Error at 'super': Can't use 'super' outside of a class.\n");
}

TEST(Super, reassign) {
    test_cellox_program("super/reassign_super.clx", "yes\nyes\n");
}

TEST(Super, withoutDot) {
    test_failing_cellox_program("super/without_dot.clx", "[line 10] Error at ';': Expect '.' after 'super'.\n");
}

TEST(Super, withoutName) {
    test_failing_cellox_program("super/without_name.clx", "[line 5] Error at ';': Expect superclass method name.\n");
}