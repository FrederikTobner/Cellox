#include "benchmark.h"

#include <float.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "init.h"

typedef struct
{
    char const * benchmarkName;
    char const * benchmarkFileName;
    size_t executionCount;
}benchmark_config_t;

static benchmark_config_t benchmarks[] = {
    [BENCHMARK_FIBONACCI] =
    {
        .benchmarkName = "Fibonacci",
        .benchmarkFileName = "FibonacciBenchmark.clx",
        .executionCount = 3
    },
    [BENCHMARK_INSTANTIATION] =
    {
        .benchmarkName = "Instantiation",
        .benchmarkFileName = "InstantiationBenchmark.clx",
        .executionCount = 3
    },
    [BENCHMARK_NEGATE] =
    {
        .benchmarkName = "Negate",
        .benchmarkFileName = "NegateBenchmark.clx",
        .executionCount = 3
    },
    [BENCHMARK_ZOO] =
    {
        .benchmarkName = "Zoo",
        .benchmarkFileName = "ZooBenchmark.clx",
        .executionCount = 3
    }
};


static void execute(benchmark_config_t benchmark);

void benchmark_execute_all_benchmarks()
{
    printf("%15s - %10s - %10s - %10s\n",  "Name", "average", "min", "max" );
    size_t benchmarkCount = sizeof(benchmarks) / sizeof(*benchmarks);
    for (size_t i = 0; i < benchmarkCount; i++)
        execute(*(benchmarks + i));    
}

void benchmark_execute(benchmark_t benchmark)
{
    printf("%15s - %10s - %10s - %10s\n",  "Name", "average", "min", "max" );
    execute(*(benchmarks + benchmark));    
}

static void execute(benchmark_config_t benchmark)
{
    double combined_execution_duration = 0.0;
    double min_execution_duration = DBL_MAX;
    double max_execution_duration = DBL_MIN;

    char * filePath = (char *)malloc(1024);
    *filePath = '\0';
    strcat(filePath, BENCHMARK_BASE_PATH);
    strcat(filePath, benchmark.benchmarkFileName);
    filePath[strlen(BENCHMARK_BASE_PATH) + strlen(filePath)] = '\0';
    const char *args[2];
    *(args + 1) = filePath;

    char * measured_time = (char *)calloc(1024, sizeof(char));
    #ifdef _WIN32
    freopen("NUL", "a", stdout);
    #endif
    #ifdef linux
    freopen("/dev/nul", "a", stdout);
    #endif
    
    double benchmark_execution_time;
    for (size_t i = 0; i < benchmark.executionCount; i++)
    {
        // Redirect standard output to the beginnining of the buffer
        setbuf(stdout, measured_time);
        // Execute benchmark
        init_initialize(2, args);
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
    #ifdef _WIN32
    freopen("CON", "w", stdout);
    #endif
    #ifdef linux
    freopen("CON", "w", stdout);
    #endif
    printf("%15s - %9gs - %9gs - %9gs\n",  
            benchmark.benchmarkName, 
            combined_execution_duration / benchmark.executionCount,
            min_execution_duration,
            max_execution_duration);
    free(measured_time);
    free(filePath);
}