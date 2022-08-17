#ifndef CELLOX_DYNAMIC_ARRAY_H
#define CELLOX_DYNAMIC_ARRAY_H

#include "common.h"
#include "value.h"

// Type definition of the dynamic value array
typedef struct
{
    int32_t capacity;
    int32_t count;
    Value *values;
} DynamicArray;

// Dealocates the memory used by the dynamic array
void freeDynamicArray(DynamicArray *array);

// initializes the dynamic array
void initDynamicArray(DynamicArray *array);

// Adds a value to the dynamic array
void writeDynamicArray(DynamicArray *array, Value value);

#endif