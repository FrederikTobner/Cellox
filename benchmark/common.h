#ifndef CELLOX_BENCHMARK_COMMON_H_
#define CELLOX_BENCHMARK_COMMON_H_

enum benchmark_exit_code
{
    EXIT_CODE_OK = 0,
    // It can only be a layer 8 issue!
    EXIT_CODE_COMMAND_LINE_USAGE_ERROR = 64
};

#endif