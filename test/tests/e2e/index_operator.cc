#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(IndexOperator, OutOfBounds) {
    test_failing_cellox_program("index_operator/out_of_bounds.clx",
                                "accessed string out of bounds (at index 4)\n[line 2] in script\n");
}

TEST(IndexOperator, WithBool) {
    test_failing_cellox_program("index_operator/with_bool.clx",
                                "Operands must a numerical value and a string object but are a boolean value and a "
                                "string object\n[line 2] in script\n");
}

TEST(IndexOperator, WithClass) {
    test_failing_cellox_program("index_operator/with_class.clx",
                                "Operands must a numerical value and a string object but are a class object and a "
                                "string object\n[line 3] in script\n");
}

TEST(IndexOperator, WithFunction) {
    test_failing_cellox_program("index_operator/with_function.clx",
                                "Operands must a numerical value and a string object but are a closure object and a "
                                "string object\n[line 3] in script\n");
}

TEST(IndexOperator, WithNull) {
    test_failing_cellox_program("index_operator/with_null.clx",
                                "Operands must a numerical value and a string object but are a undefiened value and a "
                                "string object\n[line 2] in script\n");
}

TEST(IndexOperator, WithString) {
    test_failing_cellox_program("index_operator/with_string.clx",
                                "Operands must a numerical value and a string object but are a string object and a "
                                "string object\n[line 2] in script\n");
}

TEST(IndexOperator, ArrayGetSet) {
    test_cellox_program("index_operator/array_get_set.clx", "2\n99\n");
}

TEST(IndexOperator, ArrayOutOfBounds) {
    test_failing_cellox_program("index_operator/array_out_of_bounds.clx",
                                "accessed array out of bounds (at index 3)\n[line 2] in script\n");
}