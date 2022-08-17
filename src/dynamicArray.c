#include "dynamicArray.h"

#include "memory.h"

void freeDynamicArray(DynamicArray *array)
{
    FREE_ARRAY(Value, array->values, array->capacity);
    initDynamicArray(array);
}

void initDynamicArray(DynamicArray *array)
{
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeDynamicArray(DynamicArray *array, Value value)
{
    if (array->capacity < array->count + 1)
    {
        int32_t oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
    }
    array->values[array->count] = value;
    array->count++;
}