#ifndef CELLOX_DYNAMIC_ARRAY_H_
#define CELLOX_DYNAMIC_ARRAY_H_

#include "common.h"
#include "value.h"

// Type definition of the dynamic value array
typedef struct
{
    uint32_t capacity;
    uint32_t count;
    value_t * values;
} dynamic_value_array_t;

// Dealocates the memory used by the dynamic array
void dynamic_array_free(dynamic_value_array_t * array);

// initializes the dynamic array
void dynamic_array_init(dynamic_value_array_t * array);

// Adds a value to the dynamic array
void dynamic_array_write(dynamic_value_array_t * array, value_t value);

#endif