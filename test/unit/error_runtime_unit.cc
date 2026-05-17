#include <gtest/gtest.h>

extern "C" {
#include "backend/virtual_machine.h"
#include "language-models/object.h"
#include "language-models/value.h"
}

TEST(ErrorRuntimeUnit, ErrorValueEqualityUsesSetAndVariant) {
    // Act
    virtual_machine_init();

    object_string_t * setName = object_copy_string("FileError", 9, false);
    object_string_t * notFoundName = object_copy_string("NotFound", 8, false);
    object_string_t * permissionName = object_copy_string("PermissionDenied", 16, false);

    object_error_set_t * set = object_new_error_set(setName);
    object_error_value_t * notFoundA = object_new_error_value(set, notFoundName);
    object_error_value_t * notFoundB = object_new_error_value(set, notFoundName);
    object_error_value_t * permission = object_new_error_value(set, permissionName);

    // Assert
    EXPECT_TRUE(value_values_equal(OBJECT_VAL(notFoundA), OBJECT_VAL(notFoundB)));
    EXPECT_FALSE(value_values_equal(OBJECT_VAL(notFoundA), OBJECT_VAL(permission)));

    virtual_machine_free();
}
