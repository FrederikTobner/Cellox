#ifndef CELLOX_BENCHMARK_DYNAMIC_BENCHMARK_CONFIG_ARRAY_H_
#define CELLOX_BENCHMARK_DYNAMIC_BENCHMARK_CONFIG_ARRAY_H_

#include <stdint.h>

#include "benchmark_config.h"

//// @brief Type definition of the dynamic benchmark config array
typedef struct {
    uint32_t capacity;
    uint32_t count;
    benchmark_config_t * configs;
} dynamic_benchmark_config_array_t;

/// @brief Dealocates a dynamic benchmark config array
/// @param array The array that is freed
void dynamic_benchmark_config_array_free(dynamic_benchmark_config_array_t * array);

/// @brief Initializes a dynamic benchmark config array to the size zero
/// @param array The array that is inititialized
void dynamic_benchmark_config_array_init(dynamic_benchmark_config_array_t * array);

/// @brief Adds a value to the dynamic benchmark config array
/// @param array The array where the value is added
/// @param value The value that is added to the array
void dynamic_benchmark_config_array_write(dynamic_benchmark_config_array_t * array, benchmark_config_t value);

#endif