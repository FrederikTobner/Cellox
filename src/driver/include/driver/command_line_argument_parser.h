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
 * See the <https://www.gnu.org/licenses/gpl-3.0.html/>>GNU General Public  *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file command_line_argument_parser.h
 * @brief Header file containing the declarations regarrding the command line argument parser.
 */

#ifndef CELLOX_COMMAND_LINE_ARGUMENT_PARSER_H_
#define CELLOX_COMMAND_LINE_ARGUMENT_PARSER_H_

#include <stdbool.h>
#include <stdint.h>

/// @brief Enumerates all available command line options
typedef enum {
    OPTION_HELP,
    OPTION_VERSION,
    OPTION_COMPILE,
    OPTION_OPTIMIZATION_LEVEL,
    OPTION_STDLIB_DIR
} command_line_option_type;

/// @brief Represents the parsed command line configuration
/// @details This structure contains all information extracted from the command line arguments.
/// The parser validates option constraints at parse time and populates this structure.
typedef struct {
    /// @brief Whether the help option was specified
    bool help;
    /// @brief Whether the version option was specified
    bool version;
    /// @brief Whether the compile option was specified
    bool compile;
    /// @brief The input file path (NULL if not provided)
    char const * inputFile;
    /// @brief The optimization level (0-3), or UINT32_MAX if not specified
    uint32_t optimizationLevel;
    /// @brief Explicit stdlib directory override (NULL if not provided)
    /// @details Equivalent to setting CELLOX_STDLIB_DIR for this invocation.
    char const * stdlibDir;
} command_line_config_t;

/// @brief Parses the command line arguements that were specified when cellox was called
/// @param argc The amount of arguments that were specified by the user
/// @param argv The arguments that were spepcified by the user
/// @return Populated command line configuration structure
/// @note if the arguments could not be parsed the program exits with an exit code indicating a command-line-usage
/// error by the user
command_line_config_t command_line_argument_parser_parse(int argc, char const ** argv);

#endif