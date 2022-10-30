#ifndef CELLOX_BENCHMARK_BENCHMARK_H_
#define CELLOX_BENCHMARK_BENCHMARK_H_

typedef enum
{
    BENCHMARK_FIBONACCI,
    BENCHMARK_INSTANTIATION,
    BENCHMARK_NEGATE,
    BENCHMARK_ZOO
}benchmark_t;

void benchmark_execute_all_benchmarks();

void benchmark_execute(benchmark_t benchmark);

#endif