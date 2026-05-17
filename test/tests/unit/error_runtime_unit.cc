#include "../fixtures/vm_fixture.h"
#include <gtest/gtest.h>

extern "C" {
#include "backend/virtual_machine.h"
#include "language-models/object.h"
#include "language-models/value.h"
}

TEST_F(VirtualMachineFixture, ErrorValueEqualityUsesSetAndVariant) {
    // Arrange
    object_string_t * setName = object_copy_string("FileError", 9, false);
    object_string_t * notFoundName = object_copy_string("NotFound", 8, false);
    object_string_t * permissionName = object_copy_string("PermissionDenied", 16, false);

    object_error_set_t * set = object_new_error_set(setName);
    object_error_value_t * notFoundA = object_new_error_value(set, notFoundName);
    object_error_value_t * notFoundB = object_new_error_value(set, notFoundName);
    object_error_value_t * permission = object_new_error_value(set, permissionName);

    // Act
    bool equalNotFound = value_values_equal(OBJECT_VAL(notFoundA), OBJECT_VAL(notFoundB));
    bool notEqual = value_values_equal(OBJECT_VAL(notFoundA), OBJECT_VAL(permission));
    // Assert
    EXPECT_TRUE(equalNotFound);
    EXPECT_FALSE(notEqual);
}
