#include "benchmark_runner.h"

#include <float.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../src/init.h"

/// @brief Benchmarks that are included in the benchmarking suite
static benchmark_config_t benchmarks[] = 
{
    [BENCHMARK_EQUALITY] =
    {
        .benchmarkName = "Equality",
        .benchmarkFilePath = "Equality.clx",
        .executionCount = 3
    },
    [BENCHMARK_FIBONACCI] =
    {
        .benchmarkName = "Fibonacci",
        .benchmarkFilePath = "Fibonacci.clx",
        .executionCount = 3
    },
    [BENCHMARK_INSTANTIATION] =
    {
        .benchmarkName = "Instantiation",
        .benchmarkFilePath = "Instantiation.clx",
        .executionCount = 3
    },
    [BENCHMARK_METHOD_CALL] =
    {
        .benchmarkName = "Method Call",
        .benchmarkFilePath = "MethodCall.clx",
        .executionCount = 3
    },
    [BENCHMARK_NEGATE] =
    {
        .benchmarkName = "Negate",
        .benchmarkFilePath = "Negate.clx",
        .executionCount = 3
    },
    [BENCHMARK_PROPERTIES] =
    {
        .benchmarkName = "Properties",
        .benchmarkFilePath = "Properties.clx",
        .executionCount = 3
    },
    [BENCHMARK_STRING_EQUALITY] =
    {
        .benchmarkName = "String Equality",
        .benchmarkFilePath = "StringEquality.clx",
        .executionCount = 3
    },
    [BENCHMARK_ZOO] =
    {
        .benchmarkName = "Zoo",
        .benchmarkFilePath = "Zoo.clx",
        .executionCount = 3
    }
};


static void benchmark_runner_execute_benchmark(benchmark_config_t benchmark, bool custom);

void benchmark_runner_execute_all_predefiened()
{
    printf("%10s | %10s | %10s | %8s\n",  "average", "min", "max", "name" );
    size_t benchmarkCount = sizeof(benchmarks) / sizeof(*benchmarks);
    for (size_t i = 0; i < benchmarkCount; i++)
        benchmark_runner_execute_benchmark(*(benchmarks + i), false);    
}

void benchmark_runner_execute_predefiened(benchmark_t benchmark)
{
    printf("%10s | %10s | %10s | %8s\n",  "average", "min", "max", "name" );
    benchmark_runner_execute_benchmark(*(benchmarks + benchmark), false);    
}

void benchmark_runner_execute_custom_benchmarks(dynamic_benchmark_config_array_t * config_array)
{
    printf("%10s | %10s | %10s | %8s\n-----------|------------|------------|-----------\n",  "average", "min", "max", "name" );
    for (size_t i = 0; i < config_array->count; i++)
        benchmark_runner_execute_benchmark(config_array->configs[i], true);    
}

static void benchmark_runner_execute_benchmark(benchmark_config_t benchmark, bool custom)
{
    double combined_execution_duration = 0.0;
    double min_execution_duration = DBL_MAX;
    double max_execution_duration = DBL_MIN;
    char * filePath = NULL;
    if(!custom)
    {
        filePath = (char *)malloc(strlen(BENCHMARK_BASE_PATH) + strlen(benchmark.benchmarkFilePath) + 1);
        *filePath = '\0';
        strcat(filePath, BENCHMARK_BASE_PATH);
        strcat(filePath, benchmark.benchmarkFilePath);
        filePath[strlen(BENCHMARK_BASE_PATH) + strlen(filePath)] = '\0';
    }
    else
        filePath = (char *)benchmark.benchmarkFilePath;  

    char * measured_time = (char *)calloc(1024, sizeof(char));
    #ifdef _WIN32
    freopen("NUL", "a", stdout);
    #elif __unix__
    freopen("/dev/nul", "a", stdout);
    #endif
    
    double benchmark_execution_time;
    for (size_t i = 0; i < benchmark.executionCount; i++)
    {
        // Redirect standard output to the beginnining of the buffer
        setbuf(stdout, measured_time);            
        // Execute benchmark 🚀
        init_run_from_file(filePath, false);
        // Remove newline at the end of the buffer
        measured_time[strlen(measured_time) - 1] = '\0';
        // store result
        benchmark_execution_time = atof(measured_time);
        combined_execution_duration += benchmark_execution_time;
        if(benchmark_execution_time < min_execution_duration)
            min_execution_duration = benchmark_execution_time;
        if(benchmark_execution_time > max_execution_duration)
            max_execution_duration = benchmark_execution_time;
        // Clear buffer 
        for (char * buffer = measured_time;  *buffer; buffer++)
            *buffer = '\0';
    }
    // Reset stdout redirection
    freopen("CON", "w", stdout);
    printf("%9gs | %9gs | %9gs | %s\n", 
            combined_execution_duration / benchmark.executionCount,
            min_execution_duration,
            max_execution_duration,
            benchmark.benchmarkName);
    if(!custom)
    free(measured_time);
}