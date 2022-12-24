#ifndef CELLOX_BENCHMARK_CONFIG_H_
#define CELLOX_BENCHMARK_CONFIG_H_

#include <stddef.h>

/// @brief A Benchmark configuration that can be used to execute a benchmark using the benchmarkrunner
typedef struct
{
    char const * benchmarkName;
    char const * benchmarkFilePath;
    size_t executionCount;
}benchmark_config_t;

/// @brief Indexes of the benchmarks included in the benchmarking suite
typedef enum
{
    BENCHMARK_EQUALITY,
    BENCHMARK_FIBONACCI,
    BENCHMARK_INSTANTIATION,
    BENCHMARK_METHOD_CALL,
    BENCHMARK_NEGATE,
    BENCHMARK_PROPERTIES,
    BENCHMARK_RAISE,
    BENCHMARK_ZOO
} benchmark;

#endif