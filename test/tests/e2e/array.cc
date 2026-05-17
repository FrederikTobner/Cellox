#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Array, ChangeByIndex) {
    test_cellox_program("array/change_by_index.clx", "12\n");
}

TEST(Array, EqualOperator) {
    test_cellox_program("array/equal_operator.clx", "true\n");
}

TEST(Array, GetByIndex) {
    test_cellox_program("array/get_by_index.clx", "4\n");
}

TEST(Array, PlusOperator) {
    test_cellox_program("array/plus_operator.clx", "{1, 2, true, -10, null}\n{1, 2, true, -10}\n");
}