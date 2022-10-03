#include "dynamic_array.h"

#include "memory.h"

void dynamic_array_free(DynamicArray * array)
{
    FREE_ARRAY(Value, array->values, array->capacity);
    dynamic_array_init(array);
}

void dynamic_array_init(DynamicArray * array)
{
    array->values = NULL;
    array->capacity = 0u;
    array->count = 0u;
}

void dynamic_array_write(DynamicArray * array, Value value)
{
    if (array->capacity < array->count + 1u)
    {
        uint32_t oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
    }
    array->values[array->count] = value;
    array->count++;
}