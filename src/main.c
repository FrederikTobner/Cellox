#include <stdio.h>
#include <string.h>

#include "command_line_argument_parser.h"
#include "common.h"

/// @brief Main entry point of the cellox interpreter
/// @param argc The amount of arguments that were specified by the user
/// @param argv The arguments that were spepcified by the user
/// @return 0 if no error occurs
int main(int argc, char const ** argv)
{    
    command_line_argument_parser_parse(argc, argv);
    return EXIT_CODE_OK;
}
