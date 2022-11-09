#include "command_line_argument_parser.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "benchmark_runner.h"
#include "common.h"

#define DEFAULT_EXECUTION_COUNT (10)

static void command_line_argument_parser_change_current_option_type(option_type_t, option_type_t *);
static void command_line_argument_parser_error(char const *, ...);
static inline bool command_line_argument_parser_is_option(char const *);
static void command_line_argument_parser_parse_argument(char const **, option_type_t, benchmark_config_t *, int, bool *);
static option_type_t command_line_argument_parser_parse_option(char const *);
static inline void command_line_argument_parser_show_usage();

void command_line_argument_parser_parse(int argc, char const ** argv)
{
    bool countSpecified = false;
    option_type_t currentType = OPTION_TYPE_PATH;
    benchmark_config_t * configs;
    size_t configCount;
    if(!strcmp(argv[argc - 2], "-c"))
    {
        configCount = argc - 3;
        configs = malloc(configCount * sizeof(benchmark_config_t));        
    }
    else
    {
        configCount = argc - 1;
        configs = malloc(configCount * sizeof(benchmark_config_t));
    }
    for (int i = 1; i < argc; i++)
    {
        if(command_line_argument_parser_is_option(argv[i]))
            command_line_argument_parser_change_current_option_type(command_line_argument_parser_parse_option(argv[i]), &currentType);
        else
            command_line_argument_parser_parse_argument(argv, currentType, configs, i, &countSpecified);
    }
    benchmark_runner_execute_custom_benchmarks(configs, configCount);
    free(configs);
}

static inline void command_line_argument_parser_show_usage()
{
    command_line_argument_parser_error("Usage: CelloxBenchmarks [path] [-c count]");
}

static void command_line_argument_parser_change_current_option_type(option_type_t type, option_type_t * currentType)
{
    if(type == (*currentType))
        command_line_argument_parser_error("The option was already specified!");
    if(type < (*currentType))
        command_line_argument_parser_error("Option has lower precedence");
    *currentType = type;
}

static void command_line_argument_parser_error(char const * format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
    exit(EXIT_CODE_COMMAND_LINE_USAGE_ERROR);
}

static void command_line_argument_parser_parse_argument(char const ** argv, option_type_t currentType, benchmark_config_t * configs, int index, bool * countSpecified)
{
    if(currentType == OPTION_TYPE_COUNT)
    {
        if(*countSpecified)
            command_line_argument_parser_error("Count was specified more than once!");
        int count = atoi(argv[index]);
        if(!count)
            command_line_argument_parser_show_usage();
        for (size_t i = 0; i < index - 1; i++)
            configs[i].executionCount = count;
        *countSpecified = true;
        return;
    }
    configs[index - 1].benchmarkFilePath = argv[index];
    // Enhance name
    char * name = (char *)argv[index];
    for (char * cp = name; *cp; cp++)
        if(*cp == '\\')
            name = cp + 1;    
    
    configs[index - 1].benchmarkName = name;
    configs[index - 1].executionCount = DEFAULT_EXECUTION_COUNT;        
}

static option_type_t command_line_argument_parser_parse_option(char const * option)
{
    if(!strcmp(option, "-c"))
        return OPTION_TYPE_COUNT; 
    command_line_argument_parser_error("Unknown option %s", option);
}

static inline bool command_line_argument_parser_is_option(char const * argument)
{
    return argument[0] == '-';
}