#ifndef CELLOX_COMMAND_LINE_ARGUMENT_PARSER_H_
#define CELLOX_COMMAND_LINE_ARGUMENT_PARSER_H_

/// @brief Parses the command line arguements that were specified when cellox was called
/// @param argc The amount of arguments that were specified by the user
/// @param argv The arguments that were spepcified by the user
void command_line_argument_parser_parse(int argc, char const ** argv);

#endif