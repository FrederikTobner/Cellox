#ifndef CELLOX_BENCHMARK_BENCHMARK_H_
#define CELLOX_BENCHMARK_BENCHMARK_H_

#include <stddef.h>

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

typedef struct
{
    char const * benchmarkName;
    char const * benchmarkFilePath;
    size_t executionCount;
}benchmark_config_t;

void benchmark_runner_execute_all_predefiened();

void benchmark_runner_execute_predefiened(benchmark_t benchmark);

void benchmark_runner_execute_custom_benchmarks(benchmark_config_t * config, size_t count);

#endif