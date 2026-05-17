#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(Modules, RelativeImport) {
    test_cellox_program("modules/relative_import.clx", "42\n");
}

TEST(Modules, NamedImport) {
    test_cellox_program("modules/named_import.clx", "3\n");
}

TEST(Modules, ModuleIsLoadedOnce) {
    test_cellox_program("modules/import_once.clx", "shared\n");
}
