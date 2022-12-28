#include "benchmark_runner.h"

#include <errno.h>
#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef WIN32
    #include <fileapi.h>
    #include <windows.h>
#endif

#ifdef __unix__
    #include <dirent.h> 
    #include <sys/stat.h>
#endif

#include "../src/init.h"

#include "common.h"

/// @brief Benchmarks that are included in the benchmarking suiteby default
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
    [BENCHMARK_RAISE] =
    {
        .benchmarkName = "Raise",
        .benchmarkFilePath = "Raise.clx",
        .executionCount = 3
    },
    [BENCHMARK_ZOO] =
    {
        .benchmarkName = "Zoo",
        .benchmarkFilePath = "Zoo.clx",
        .executionCount = 3
    }
};

static FILE * benchmark_runner_create_results_file_pointer();
static void benchamrk_runner_ensure_results_directory_exists();
static void benchmark_runner_execute_benchmark(benchmark_config_t, bool, FILE *);

void benchmark_runner_execute_all_predefiened()
{
    FILE * filePointer = benchmark_runner_create_results_file_pointer();
    printf("%10s | %10s | %10s | %8s\n",  "average", "min", "max", "name");
    fprintf(filePointer, "%10s | %10s | %10s | %8s\n",  "average", "min", "max", "name");
    size_t benchmarkCount = sizeof(benchmarks) / sizeof(*benchmarks);
    for (size_t i = 0; i < benchmarkCount; i++)
        benchmark_runner_execute_benchmark(*(benchmarks + i), false, filePointer);
    fclose(filePointer);
}

void benchmark_runner_execute_predefiened(benchmark benchmark)
{
    FILE * filePointer = benchmark_runner_create_results_file_pointer();
    printf("%10s | %10s | %10s | %8s\n",  "average", "min", "max", "name");
    fprintf(filePointer, "%10s | %10s | %10s | %8s\n",  "average", "min", "max", "name");
    benchmark_runner_execute_benchmark(*(benchmarks + benchmark), false, filePointer);
    fclose(filePointer);
}

void benchmark_runner_execute_custom_benchmarks(dynamic_benchmark_config_array_t * config_array)
{
    FILE * filePointer = benchmark_runner_create_results_file_pointer();
    printf("%10s | %10s | %10s | %8s\n-----------|------------|------------|-----------\n",  "average", "min", "max", "name");
    fprintf(filePointer, "%10s | %10s | %10s | %8s\n-----------|------------|------------|-----------\n",  "average", "min", "max", "name");
    for (size_t i = 0; i < config_array->count; i++)
        benchmark_runner_execute_benchmark(config_array->configs[i], true, filePointer);
    fclose(filePointer);
}

static FILE * benchmark_runner_create_results_file_pointer()
{
    FILE * filePointer;
    time_t current_time = time(NULL);
    char * fileName = ctime(&current_time);
    if (!fileName)
    {
        printf("Unable to convert time to a character sequence.\n");
        exit(EXIT_CODE_SYSTEM_ERROR);
    }
    size_t fileNameSize = strlen(fileName);
    for (size_t i = 0; i < fileNameSize; i++)
    {
        if (fileName[i] == ' ')
            fileName[i] = '_';
        else if (fileName[i] == ':')
            fileName[i] = '-';
    }
    fileName = realloc(fileName, fileNameSize + 5);
    if (!fileName)
    {
        fprintf(stderr, "Unable to allocate memory for the filename.\n");
        exit(EXIT_CODE_SYSTEM_ERROR);
    }
    fileName[fileNameSize + 4] = '\0';
    fileName[fileNameSize + 3] = 'm';
    fileName[fileNameSize + 2] = 'b';
    fileName[fileNameSize + 1] = 'x';
    fileName[fileNameSize] = 'c';
    fileName[fileNameSize - 1] = '.';
    char * fullFileNamePath;
    if((fullFileNamePath = malloc(strlen(fileName)+strlen(RESULTS_BASE_PATH) + 2)) != NULL)
    {
        fullFileNamePath[0] = '\0';   // ensures the memory is an empty string
        strcat(fullFileNamePath, RESULTS_BASE_PATH);
        strcat(fullFileNamePath, "/");
        strcat(fullFileNamePath, fileName);
    } 
    else 
    {
        fprintf(stderr, "Could not allocate memory needed for full file name path!\n");
        exit(EXIT_CODE_SYSTEM_ERROR);
    }
    benchamrk_runner_ensure_results_directory_exists();
    filePointer = fopen(fullFileNamePath, "w");
    if (!filePointer)
    {
        fprintf(stderr, "Unable to create file.\n");
        exit(EXIT_CODE_SYSTEM_ERROR);
    }
    free(fileName);
    free(fullFileNamePath);
    return filePointer;
}

static void benchamrk_runner_ensure_results_directory_exists()
{
    #ifdef WIN32
        DWORD dwAttribute = GetFileAttributes(RESULTS_BASE_PATH);
        if(dwAttribute == INVALID_FILE_ATTRIBUTES)
        {
            CreateDirectory(RESULTS_BASE_PATH, NULL);
            dwAttribute = GetFileAttributes(RESULTS_BASE_PATH);
            if(dwAttribute == INVALID_FILE_ATTRIBUTES)
            {
                fprintf(stderr, "Can not create results directory!\n");
                exit(EXIT_CODE_SYSTEM_ERROR);
            }
        }
        else if(!(dwAttribute & FILE_ATTRIBUTE_DIRECTORY))
        {
            fprintf(stderr, "Results is not a directory");
            exit(EXIT_CODE_SYSTEM_ERROR);
        }
    #endif
    #ifdef __unix__
    DIR * resultsDirectory = opendir(RESULTS_BASE_PATH);
    if (resultsDirectory) 
        closedir(resultsDirectory);
    else if (ENOENT == errno) 
    {
        // If the directory does not exists we need to create it
        mkdir(RESULTS_BASE_PATH, 0700);
        resultsDirectory = opendir(RESULTS_BASE_PATH);
        if(resultsDirectory)
            closedir(resultsDirectory);
        else
        {
            fprintf(stderr, "Can not create results directory!\n");
            exit(EXIT_CODE_SYSTEM_ERROR);
        }

    } 
    else 
    {
        fprintf(stderr, "Can not access results directory!\n");
        exit(EXIT_CODE_SYSTEM_ERROR);
    }
    #endif
}

static void benchmark_runner_execute_benchmark(benchmark_config_t benchmark, bool custom, FILE * filePointer)
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
    fprintf(filePointer, "%9gs | %9gs | %9gs | %s\n", 
            combined_execution_duration / benchmark.executionCount,
            min_execution_duration,
            max_execution_duration,
            benchmark.benchmarkName);
    if(!custom)
        free(filePath);
    free(measured_time);
}