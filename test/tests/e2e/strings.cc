#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Strings, ChangeStringByIndex) {
    test_cellox_program("strings/change_by_index.clx", "celiop\ncellop\ncellox\n");
}

TEST(Strings, GetByIndex) {
    test_cellox_program("strings/get_by_index.clx", "t\ne\ns\nt\n");
}

TEST(Strings, ConcatenationEdgeCases) {
    test_cellox_program("strings/concatenation_edge_cases.clx", "\nab\n");
}