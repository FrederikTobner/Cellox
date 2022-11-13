#include <stdio.h>
#include <string.h>

#include "common.h"
#include "init.h"
#include "virtual_machine.h"

/// @brief Main entry point of the cellox interpreter
/// @param argc The amount of arguments that were specified by the user
/// @param argv The arguments that were spepcified by the user
/// @return 0 -> OK, 64 -> wrong usage, 70 -> runtime error, 74 error reading from file, 80 memory allocation error
int main(int argc, char const ** argv)
{
    
    if (argc == 1)
        init_repl();
    else if (argc == 2)
        init_run_from_file(argv[1], false);
    else if(argc == 3 && !strcmp("-s", argv[1]))
        init_run_from_file(argv[2], true);
    else
    {
        // Too much arguments (>1) TODO: Add argumenrs for the compiler e.g. --analyze/-a, --help, --store/-s and --version/-v options 
        fprintf(stderr, "Usage: Cellox (-s|-r)? [path]\n");
        return EXIT_CODE_COMMAND_LINE_USAGE_ERROR;
    }
    return EXIT_CODE_OK;
}
