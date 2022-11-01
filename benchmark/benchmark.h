#ifndef CELLOX_BENCHMARK_BENCHMARK_H_
#define CELLOX_BENCHMARK_BENCHMARK_H_

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

void benchmark_execute_all_benchmarks();

void benchmark_execute(benchmark_t benchmark);

#endif