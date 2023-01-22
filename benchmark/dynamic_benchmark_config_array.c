#include "dynamic_benchmark_config_array.h"

#include <stdlib.h>

#include "../src/backend/memory_mutator.h"

void dynamic_benchmark_config_array_free(dynamic_benchmark_config_array_t * array)
{
    FREE_ARRAY(benchmark_config_t, array->configs, array->capacity);
    dynamic_benchmark_config_array_init(array);
}

void dynamic_benchmark_config_array_init(dynamic_benchmark_config_array_t * array)
{
    array->configs = NULL;
    array->count = array->capacity = 0u;
}

void dynamic_benchmark_config_array_write(dynamic_benchmark_config_array_t * array, benchmark_config_t benchmark)
{
    if (array->capacity < array->count + 1u)
    {
        uint32_t oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        benchmark_config_t * grownArray;
        grownArray = GROW_ARRAY(benchmark_config_t, array->configs, oldCapacity, array->capacity);
        array->configs = grownArray;
    }
    array->configs[array->count] = benchmark;
    array->count++;
}
