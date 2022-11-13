#ifndef CELLOX_BENCHMARK_CONFIG_H_
#define CELLOX_BENCHMARK_CONFIG_H_

#include <stddef.h>

typedef struct
{
    char const * benchmarkName;
    char const * benchmarkFilePath;
    size_t executionCount;
}benchmark_config_t;

typedef enum
{
    BENCHMARK_EQUALITY,
    BENCHMARK_FIBONACCI,
    BENCHMARK_INSTANTIATION,
    BENCHMARK_METHOD_CALL,
    BENCHMARK_NEGATE,
    BENCHMARK_PROPERTIES,
    BENCHMARK_STRING_EQUALITY,
    BENCHMARK_ZOO
}benchmark_t;

#endif