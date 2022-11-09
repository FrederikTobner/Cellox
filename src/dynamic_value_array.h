#ifndef CELLOX_DYNAMIC_ARRAY_H_
#define CELLOX_DYNAMIC_ARRAY_H_

#include "common.h"
#include "value.h"

//// @brief Type definition of the dynamic value array
typedef struct
{
    uint32_t capacity;
    uint32_t count;
    value_t * values;
} dynamic_value_array_t;

/// @brief Dealocates a dynamic array
/// @param array The array that is freed
void dynamic_value_array_free(dynamic_value_array_t * array);

/// @brief Initializes a dynamic array to the size zero
/// @param array The array that is inititialized
void dynamic_value_array_init(dynamic_value_array_t * array);

/// @brief Adds a value to the dynamic array
/// @param array The array where the value is added
/// @param value The value that is added to the array
void dynamic_value_array_write(dynamic_value_array_t * array, value_t value);

#endif