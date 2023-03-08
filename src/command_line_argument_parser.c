/****************************************************************************
 * Copyright (C) 2022 by Frederik Tobner                                    *
 *                                                                          *
 * This file is part of Cellox.                                             *
 *                                                                          *
 * Permission to use, copy, modify, and distribute this software and its    *
 * documentation under the terms of the GNU General Public License is       *
 * hereby granted.                                                          *
 * No representations are made about the suitability of this software for   *
 * any purpose.                                                             *
 * It is provided "as is" without express or implied warranty.              *
 * See the <https://www.gnu.org/licenses/gpl-3.0.html/>GNU General Public   *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file command_line_argument_parser.c
 * @brief File containing the implementation of the command line argument parser.
 */

#include "command_line_argument_parser.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "initializer.h"

/// @brief Command line options of the cellox compiler
typedef enum {
    /// No option specified (yet)
    OPTION_NO_OPTION,
    /// --compile / -c
    OPTION_TYPE_COMPILE,
    /// --help / -h
    OPTION_TYPE_HELP,
    /// --version / -v
    OPTION_TYPE_VERSION
} command_line_option_type;

/// @brief Models a command line option configuration
typedef struct {
    /// @brief The short representation of the option
    /// @details E.g. for the help option that would be "-h"
    char const * shortRepresentation;
    /// @brief The short representation of the option
    /// @details E.g. for the help option that would be "--help"
    char const * longRepresentation;
    /// Boolean value that determines whether the option is exclusionary (can not be combined with other options)
    bool exclusionaryOption;
} command_line_option_type_config_t;

/// @brief Option configurations for all the possible options
static command_line_option_type_config_t optionConfigs[] = {
    [OPTION_NO_OPTION] = {.exclusionaryOption = false},
    [OPTION_TYPE_COMPILE] = {.shortRepresentation = "-c", .longRepresentation = "--compile", .exclusionaryOption = true},
    [OPTION_TYPE_HELP] = {.shortRepresentation = "-h", .longRepresentation = "--help", .exclusionaryOption = true},
    [OPTION_TYPE_VERSION] = {.shortRepresentation = "-v", .longRepresentation = "--version", .exclusionaryOption = true}};

static void command_line_argument_parser_error(char const *, ...);
static inline bool command_line_argument_parser_is_option(char const *);
static void command_line_argument_parser_parse_option(char const *, command_line_option_type *);
static inline void command_line_argument_parser_show_usage();

void command_line_argument_parser_parse(int argc, char const ** argv) {
    command_line_option_type currentOption = OPTION_NO_OPTION;
    for (int i = 1; i < argc; i++) {
        if (command_line_argument_parser_is_option(argv[i])) {
            command_line_argument_parser_parse_option(argv[i], &currentOption);
        } else {
            // Only a single argument is supported at the moment
            if (i + 1 != argc) {
                command_line_argument_parser_show_usage();
            } else {
                switch (currentOption) {
                case OPTION_NO_OPTION:
                    initializer_run_from_file(argv[i], false);
                    return;
                case OPTION_TYPE_COMPILE:
                    initializer_run_from_file(argv[i], true);
                    return;
                default:
                    command_line_argument_parser_show_usage();
                }
            }
        }
    }
    switch (currentOption) {
    case OPTION_TYPE_HELP:
        initializer_show_help();
        break;
    case OPTION_TYPE_VERSION:
        initializer_show_version();
        break;
    case OPTION_NO_OPTION:
        initializer_run_as_repl();
        break;
    default:
        command_line_argument_parser_show_usage();
    }
}

/// @brief Emits an error and exits the program with the appropriate exit code
/// @param format The format of the message that is printed
/// @param ... The arguments that are printed using the previously specified format
static void command_line_argument_parser_error(char const * format, ...) {
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
static inline bool command_line_argument_parser_is_option(char const * argument) {
    return argument[0] == '-';
}

/// @brief Parses the next option in the arguments
/// @param option The option that is parsed (character sequence)
/// @param currentOption The option that was previously specified
static void command_line_argument_parser_parse_option(char const * option, command_line_option_type * currentOption) {
    // Old option is a singular option
    if (optionConfigs[*currentOption].exclusionaryOption) {
        command_line_argument_parser_error("Multiple options specified");
    }
    size_t upperBound = sizeof(optionConfigs) / sizeof(command_line_option_type_config_t);
    for (size_t i = 1; i < upperBound; i++) {
        if (!strcmp(optionConfigs[i].shortRepresentation, option) || !strcmp(optionConfigs[i].longRepresentation, option)) {
            // New option is a singular option
            if (optionConfigs[i].exclusionaryOption && *currentOption) {
                command_line_argument_parser_error("Multiple exclusionary options specified");
            }
            *currentOption = i;
            break;
        }
    }
}

/// @brief Shows a brief explanation how the compiler can be used from the command line
/// @note Also exits the program with a command-line-usage error exit code
static inline void command_line_argument_parser_show_usage() {
    command_line_argument_parser_error(CELLOX_USAGE_MESSAGE);
}