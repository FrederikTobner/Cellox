#include <gtest/gtest.h>

#include "test_cellox.hh"

TEST(ErrorHandling, ErrorSetBasic) {
    test_cellox_program("error_handling/error_set_basic.clx",
                        "FileError.NotFound\nFileError.PermissionDenied\ntrue\nfalse\n");
}

TEST(ErrorHandling, MustSuccess) {
    test_cellox_program("error_handling/must_success.clx", "5\n");
}

TEST(ErrorHandling, CatchBasic) {
    test_cellox_program("error_handling/catch_basic.clx", "5\n-1\n");
}

TEST(ErrorHandling, IferrorBasic) {
    test_cellox_program("error_handling/iferror_basic.clx",
                        "error: FileError.NotFound\nok: readme.txt\n");
}

TEST(ErrorHandling, TryBasic) {
    test_cellox_program("error_handling/try_basic.clx", "5\npropagated\n");
}

TEST(ErrorHandling, MustPlainValue) {
    test_cellox_program("error_handling/must_plain_value.clx", "42\nhello\n");
}

TEST(ErrorHandling, MustRuntimeError) {
    test_failing_cellox_program("error_handling/must_runtime_error.clx",
                                "must: unhandled error MathError.DivByZero\n[line 10] in script\n");
}

TEST(ErrorHandling, CatchChained) {
    test_cellox_program("error_handling/catch_chained.clx", "default\nok\n");
}

TEST(ErrorHandling, TryMultilevel) {
    test_cellox_program("error_handling/try_multilevel.clx", "data.txt\nmissing\n");
}

TEST(ErrorHandling, TryPlainValue) {
    test_cellox_program("error_handling/try_plain_value.clx", "7\n");
}

TEST(ErrorHandling, IferrorPlainValue) {
    test_cellox_program("error_handling/iferror_plain_value.clx", "ok: data\n");
}

TEST(ErrorHandling, ThrowNestedScope) {
    test_cellox_program("error_handling/throw_nested_scope.clx", "10\n-1\n");
}

TEST(ErrorHandling, MultipleErrorSets) {
    test_cellox_program("error_handling/multiple_error_sets.clx", "true\nfalse\ntrue\n");
}

TEST(ErrorHandling, IferrorErrValue) {
    test_cellox_program("error_handling/iferror_err_value.clx", "true\nfalse\n");
}

TEST(ErrorHandling, TryPreservesErrorType) {
    test_cellox_program("error_handling/try_preserves_error_type.clx", "true\n");
}

TEST(ErrorHandling, StdlibIoErrors) {
    test_cellox_program("error_handling/stdlib_io_errors.clx", "missing\nbadarg\ntrue\nfalse\n");
}

TEST(ErrorHandling, StdlibIoSuccess) {
    test_cellox_program("error_handling/stdlib_io_success.clx", "true\ntrue\nhello!\n");
}

TEST(ErrorHandling, StdlibNonIoErrors) {
    test_cellox_program("error_handling/stdlib_non_io_errors.clx", "type\ndomain\nformat\ntrue\nfalse\n");
}

TEST(ErrorHandling, StdlibArityErrors) {
    test_cellox_program("error_handling/stdlib_arity_errors.clx", "arity\narity\noknull\narity\ntrue\n");
}

TEST(ErrorHandling, ExpressionForms) {
    test_cellox_program("error_handling/expression_forms.clx",
                        "true\nok\ntrue\nok:readme\ntrue\nok2:readme\nok3:readme\ntrue\n");
}

TEST(ErrorHandling, BinderValueOnly) {
    test_cellox_program("error_handling/binder_value_only.clx", "handled\nsnapshot\nchanged\nsnapshot\niferror\n");
}

TEST(ErrorHandling, BinderScopeOutside) {
    test_failing_cellox_program("error_handling/binder_scope_outside.clx", "Undefined variable 'err'.\n[line 11] in script\n");
}
