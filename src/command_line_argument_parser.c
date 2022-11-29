#include "command_line_argument_parser.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "init.h"

/// @brief Command line options of the cellox interpreter
typedef enum
{
    /// No option specified (yet)
    OPTION_NO_OPTION,
    /// --help / -h
    OPTION_TYPE_HELP,
    /// --version / -v
    OPTION_TYPE_VERSION,
    /// --store / -s
    OPTION_TYPE_STORE_CHUNK_FILE,
    /// --run-chunk-file / -rcf
    OPTION_TYPE_RUN_CHUNK_FILE
}command_line_option_type_t;

/// @brief Models a command line option configuration
typedef struct
{
    /// @brief The short representation of the option
    /// @details E.g. for the help option that would be "-h"
    char const * shortRepresentation;
    /// @brief The short representation of the option
    /// @details E.g. for the help option that would be "--help"
    char const * longRepresentation;
    /// Boolean value that determines whether the option is exclusionary (can not be combined with other options)
    bool exclusionaryOption;
}command_line_option_type_config_t;


/// @brief Option configurations for all the possible options
static command_line_option_type_config_t optionConfigs [] = 
{
    [OPTION_NO_OPTION] = 
    {
        .exclusionaryOption = false
    },
    [OPTION_TYPE_HELP] = 
    {
        .shortRepresentation = "-h", 
        .longRepresentation = "--help",
        .exclusionaryOption = true
    },
    [OPTION_TYPE_VERSION] = 
    {
        .shortRepresentation = "-v", 
        .longRepresentation = "--version",
        .exclusionaryOption = true
    },
    [OPTION_TYPE_RUN_CHUNK_FILE] = 
    {
        .shortRepresentation = "-rcf", 
        .longRepresentation = "--run-chunk-file",
        .exclusionaryOption = true
    },
    [OPTION_TYPE_STORE_CHUNK_FILE] = 
    {
        .shortRepresentation = "-scf", 
        .longRepresentation = "--store-chunk-file",
        .exclusionaryOption = true
    }
};

static void command_line_argument_parser_error(char const *, ...);
static inline bool command_line_argument_parser_is_option(char const *);
static void command_line_argument_parser_parse_option(char const *, command_line_option_type_t *);
static inline void command_line_argument_parser_show_usage();

void command_line_argument_parser_parse(int argc, char const ** argv)
{
    command_line_option_type_t currentOption = OPTION_NO_OPTION;
    for (int i = 1; i < argc; i++)
    {
        if(command_line_argument_parser_is_option(argv[i]))
            command_line_argument_parser_parse_option(argv[i], &currentOption);
        else
        {
            // Only a single argument is supported at the moment
            if(i + 1 != argc)
                command_line_argument_parser_show_usage();
            else
                switch (currentOption)
                {
                case OPTION_NO_OPTION:
                    init_run_from_file(argv[i], false);
                    return;
                case OPTION_TYPE_STORE_CHUNK_FILE:
                    init_run_from_file(argv[i], true);
                    return;
                default:
                    command_line_argument_parser_show_usage();
                }                
        }
    }
    switch (currentOption)
    {
    case OPTION_TYPE_HELP:
        init_show_help();
        break;
    case OPTION_TYPE_VERSION:
        init_show_version();
        break;
    case OPTION_NO_OPTION:
        init_repl();
        break;
    default:
        command_line_argument_parser_show_usage();
    }
}

/// @brief Emits an error and exits the program with the appropriate exit code
/// @param format The format of the message that is printed
/// @param ... The arguments that are printed using the previously specified format
static void command_line_argument_parser_error(char const * format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
    exit(EXIT_CODE_COMMAND_LINE_USAGE_ERROR);
}

/// @brief Determines whether the argument is an option
/// @param argument The argument that is checked
/// @return true if it is an option, false if not
static inline bool command_line_argument_parser_is_option(char const * argument)
{
    return argument[0] == '-';
}

/// @brief Parses the next option in the arguments
/// @param option The option that is parsed (character sequence)
/// @param currentOption The option that was previously specified
static void command_line_argument_parser_parse_option(char const * option, command_line_option_type_t * currentOption)
{
    /// Old options is a singular option
    if(optionConfigs[*currentOption].exclusionaryOption)
        command_line_argument_parser_error("Multiple options specified");
    for (size_t i = 1; i < sizeof(optionConfigs) / sizeof(command_line_option_type_config_t); i++)
    {
        if(!strcmp(optionConfigs[i].shortRepresentation, option) || !strcmp(optionConfigs[i].longRepresentation, option))
        {
            // New option is a singular option
            if(optionConfigs[i].exclusionaryOption && *currentOption)
                command_line_argument_parser_error("Multiple exclusionary options specified");
            *currentOption = i;
            break;
        }
    }    
}

/// @brief Shows a brief explanation how the interpreter can be used from the command line
static inline void command_line_argument_parser_show_usage()
{
    command_line_argument_parser_error(CELLOX_USAGE_MESSAGE);
}