#ifndef CELLOX_BENCHMARK_BENCHMARK_H_
#define CELLOX_BENCHMARK_BENCHMARK_H_

#include <stddef.h>

#include "benchmark_config.h"
#include "dynamic_benchmark_config_array.h"

void benchmark_runner_execute_all_predefiened();

void benchmark_runner_execute_predefiened(benchmark benchmark);

void benchmark_runner_execute_custom_benchmarks(dynamic_benchmark_config_array_t * config_array);

#endif