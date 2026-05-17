#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(SliceEdgeCases, NegativeLowerBoundString) {
    test_failing_cellox_program("slice/negative_lower_bound_string.clx",
                                "LowerBound can not be negative, but is -1\n[line 2] in script\n");
}

TEST(SliceEdgeCases, UpperBoundEqualsStringLength) {
    test_failing_cellox_program(
        "slice/upper_bound_equals_string_length.clx",
        "Upperbound can not be higher or equal to the length of the string but upperbound is 4 and size 4\n"
        "[line 2] in script\n");
}

TEST(SliceEdgeCases, UpperBoundEqualsArrayLength) {
    test_failing_cellox_program(
        "slice/upper_bound_equals_array_length.clx",
        "Upperbound can not be higher or equal to the size of the array, but upperbound is 3 and size 3\n"
        "[line 2] in script\n");
}
