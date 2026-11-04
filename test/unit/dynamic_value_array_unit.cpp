#include <gtest/gtest.h>

extern "C" {
#include "language-models/dynamic_value_array.h"
}

TEST(DynamicValueArrayUnit, RemoveMiddleElementShiftsTail) {
    // Arrange
    dynamic_value_array_t array;
    dynamic_value_array_init(&array);

    // Act
    dynamic_value_array_write(&array, NUMBER_VAL(1));
    dynamic_value_array_write(&array, NUMBER_VAL(2));
    dynamic_value_array_write(&array, NUMBER_VAL(3));

    dynamic_value_array_remove(&array, 1);

    // Assert
    ASSERT_EQ(2u, array.count);
    EXPECT_DOUBLE_EQ(1.0, AS_NUMBER(array.values[0]));
    EXPECT_DOUBLE_EQ(3.0, AS_NUMBER(array.values[1]));

    dynamic_value_array_free(&array);
}

TEST(DynamicValueArrayUnit, RemoveOutOfBoundsKeepsArrayUnchanged) {
    // Arrange
    dynamic_value_array_t array;
    dynamic_value_array_init(&array);

    // Act
    dynamic_value_array_write(&array, NUMBER_VAL(7));
    dynamic_value_array_remove(&array, 5);

    // Assert
    ASSERT_EQ(1u, array.count);
    EXPECT_DOUBLE_EQ(7.0, AS_NUMBER(array.values[0]));

    dynamic_value_array_free(&array);
}

TEST(DynamicValueArrayUnit, RemoveFirstElementShiftsAllRemaining) {
    // Arrange
    dynamic_value_array_t array;
    dynamic_value_array_init(&array);

    // Act
    dynamic_value_array_write(&array, NUMBER_VAL(10));
    dynamic_value_array_write(&array, NUMBER_VAL(20));
    dynamic_value_array_write(&array, NUMBER_VAL(30));

    dynamic_value_array_remove(&array, 0);

    // Assert
    ASSERT_EQ(2u, array.count);
    EXPECT_DOUBLE_EQ(20.0, AS_NUMBER(array.values[0]));
    EXPECT_DOUBLE_EQ(30.0, AS_NUMBER(array.values[1]));

    dynamic_value_array_free(&array);
}

TEST(DynamicValueArrayUnit, RemoveLastElementKeepsPrefix) {
    // Arrange
    dynamic_value_array_t array;
    dynamic_value_array_init(&array);

    // Act
    dynamic_value_array_write(&array, NUMBER_VAL(10));
    dynamic_value_array_write(&array, NUMBER_VAL(20));
    dynamic_value_array_write(&array, NUMBER_VAL(30));

    dynamic_value_array_remove(&array, 2);

    // Assert
    ASSERT_EQ(2u, array.count);
    EXPECT_DOUBLE_EQ(10.0, AS_NUMBER(array.values[0]));
    EXPECT_DOUBLE_EQ(20.0, AS_NUMBER(array.values[1]));

    dynamic_value_array_free(&array);
}
