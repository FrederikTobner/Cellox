#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Methods, ArityViolated) {
    test_failing_cellox_program("method/arrity_violated.clx", "Expected 2 arguments but got 4.\n[line 8] in script\n");
}

TEST(Methods, BindThis) {
    test_cellox_program("method/binds_this.clx", "foo1\n1\n");
}

TEST(Methods, Empty) {
    test_cellox_program("method/empty.clx", "null\n");
}

TEST(Methods, GetAndSet) {
    test_cellox_program("method/get_and_set.clx", "other\n1\nmethod\n2\n");
}

TEST(Methods, MissingArguments) {
    test_failing_cellox_program("method/missing_arguments.clx",
                                "Expected 2 arguments but got 1.\n[line 5] in script\n");
}

TEST(Methods, Simple) {
    test_cellox_program("method/simple.clx", "arg\n");
}