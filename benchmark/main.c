#include "benchmark.h"

/// @brief Main entry point of the cellox benchmarking suite
/// @param argc The amount of arguments that were specified by the user - unused
/// @param argv The arguments that were spepcified by the user - unused
/// @return 0 -> OK, 64 -> wrong usage, 70 -> runtime error, 74 error reading from file, 80 memory allocation error
int main(int argc, char const ** argv)
{
    benchmark_execute(BENCHMARK_NEGATE);
    return 0;
}
