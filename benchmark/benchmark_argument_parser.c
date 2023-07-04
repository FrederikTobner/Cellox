#include "benchmark_argument_parser.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "benchmark_runner.h"
#include "common.h"
#include "dynamic_benchmark_config_array.h"

#define DEFAULT_EXECUTION_COUNT (10)

static void benchmark_argument_parser_change_current_option_type(option_type, option_type *);
static void benchmark_argument_parser_error(char const *, ...);
static inline bool benchmark_argument_parser_is_option(char const *);
static void benchmark_argument_parser_parse_argument(char const **, option_type, dynamic_benchmark_config_array_t *,
                                                     int, bool *);
static option_type benchmark_argument_parser_parse_option(char const *);
static inline void benchmark_argument_parser_show_usage(void);

void benchmark_argument_parser_parse(int argc, char const ** argv) {
    bool countSpecified = false;
    option_type currentType = OPTION_TYPE_PATH;
    dynamic_benchmark_config_array_t config_array;
    dynamic_benchmark_config_array_init(&config_array);
    for (int i = 1; i < argc; i++) {
        if (benchmark_argument_parser_is_option(argv[i])) {
            benchmark_argument_parser_change_current_option_type(benchmark_argument_parser_parse_option(argv[i]),
                                                                 &currentType);
        } else {
            benchmark_argument_parser_parse_argument(argv, currentType, &config_array, i, &countSpecified);
        }
    }
    benchmark_runner_execute_custom_benchmarks(&config_array);
    dynamic_benchmark_config_array_free(&config_array);
}

static inline void benchmark_argument_parser_show_usage(void) {
    benchmark_argument_parser_error("Usage: CelloxBenchmarks [path] [-c count]");
}

static void benchmark_argument_parser_change_current_option_type(option_type type, option_type * currentType) {
    if (type == (*currentType)) {
        benchmark_argument_parser_error("The option was already specified!");
    }
    if (type < (*currentType)) {
        benchmark_argument_parser_error("Option has lower precedence");
    }
    *currentType = type;
}

static void benchmark_argument_parser_error(char const * format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
    exit(EXIT_CODE_COMMAND_LINE_USAGE_ERROR);
}

static void benchmark_argument_parser_parse_argument(char const ** argv, option_type currentType,
                                                     dynamic_benchmark_config_array_t * config_array, int index,
                                                     bool * countSpecified) {
    if (currentType == OPTION_TYPE_COUNT) {
        if (*countSpecified) {
            benchmark_argument_parser_error("Count was specified more than once!");
        }
        int count = atoi(argv[index]);
        if (!count) {
            benchmark_argument_parser_show_usage();
        }
        for (size_t i = 0; i < index - 1; i++) {
            config_array->configs[i].executionCount = count;
        }
        *countSpecified = true;
        return;
    }

// Make sure benchmark file path is a cellox file
#ifdef OS_WINDOWS
    if (!strcmp(argv[index] + strlen(argv[index]) - 5, ".clx")) {
        benchmark_argument_parser_error("Benchmark runner can only run custom benchmarks from a cellox file");
    }
#endif

    // Enhance name
    char * name = (char *)argv[index];
    for (char * cp = (char *)argv[index]; *cp; cp++) {
        if (*cp == '\\' || *cp == '/') {
            name = cp + 1;
        }
    }

    benchmark_config_t newConfig = {
        .benchmarkFilePath = argv[index], .benchmarkName = name, .executionCount = DEFAULT_EXECUTION_COUNT};
    dynamic_benchmark_config_array_write(config_array, newConfig);
}

static option_type benchmark_argument_parser_parse_option(char const * option) {
    if (!strcmp(option, "-c")) {
        return OPTION_TYPE_COUNT;
    }
    benchmark_argument_parser_error("Unknown option %s", option);
    return OPTION_TYPE_PATH; // Unreachable
}

static inline bool benchmark_argument_parser_is_option(char const * argument) {
    return argument[0] == '-';
}