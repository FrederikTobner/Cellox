#include "benchmark_argument_parser.h"
#include "benchmark_runner.h"
#include "common.h"

/// @brief Main entry point of the cellox benchmarking suite
/// @param argc The amount of arguments that were specified by the user - unused
/// @param argv The arguments that were spepcified by the user - unused
/// @return 0 -> OK, 64 -> wrong arguments (command line usage error), 70 -> runtime error (internal software error), 74
/// error reading from file (I/O error), 71 memory allocation error (system error)
int main(int argc, char const ** argv) {
    if (argc == 1) {
        benchmark_runner_execute_all_predefiened();
    } else {
        benchmark_argument_parser_parse(argc, argv);
    }
    return EXIT_CODE_OK;
}
