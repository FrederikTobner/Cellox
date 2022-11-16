#ifndef CELLOX_BENCHMARK_BENCHMARK_ARGUMENT_PARSER_H_
#define CELLOX_BENCHMARK_BENCHMARK_ARGUMENT_PARSER_H_

typedef enum
{
    OPTION_TYPE_PATH,
    OPTION_TYPE_COUNT
}option_type_t;

void benchmark_argument_parser_parse(int argc, char const ** argv);

#endif