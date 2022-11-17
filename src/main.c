#include <stdio.h>
#include <string.h>

#include "common.h"
#include "command_line_argument_parser.h"

/// @brief Main entry point of the cellox interpreter
/// @param argc The amount of arguments that were specified by the user
/// @param argv The arguments that were spepcified by the user
/// @return 0 -> OK, 64 -> wrong usage, 70 -> runtime error, 71 -> memory allocation error, 74 -> error reading from file
int main(int argc, char const ** argv)
{    
    command_line_argument_parser_parse(argc, argv);
    return EXIT_CODE_OK;
}
