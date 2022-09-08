#ifndef cellox_dynamic_array_h
#define cellox_dynamic_array_h

#include "common.h"
#include "value.h"

// Type definition of the dynamic value array
typedef struct
{
    uint32_t capacity;
    uint32_t count;
    Value *values;
} DynamicArray;

// Dealocates the memory used by the dynamic array
void dynamic_array_free(DynamicArray *array);

// initializes the dynamic array
void dynamic_array_init(DynamicArray *array);

// Adds a value to the dynamic array
void dynamic_array_write(DynamicArray *array, Value value);

#endif