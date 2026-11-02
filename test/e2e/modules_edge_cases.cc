#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(ModulesEdgeCases, MissingModuleFile) {
    test_failing_cellox_program_contains("modules/missing_module.clx", "Could not resolve module");
}

TEST(ModulesEdgeCases, UnknownNamedExport) {
    test_failing_cellox_program_contains("modules/unknown_named_export.clx", "Unknown export");
}

TEST(ModulesEdgeCases, DuplicateExportName) {
    test_failing_cellox_program_contains("modules/duplicate_export.clx", "Invalid import or export declaration");
}

TEST(ModulesEdgeCases, CyclicImport) {
    test_failing_cellox_program_contains("modules/cycle_a.clx", "Cyclic module import detected");
}
