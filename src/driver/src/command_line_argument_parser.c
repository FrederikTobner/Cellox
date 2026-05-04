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
 *
 * @details The parser uses a type-based metadata constraint system:
 * - Option types define what can and cannot be combined:
 *   - SINGLETON: mutually exclusive with all other options
 *   - MODIFIER: can be combined with other modifiers and positional args
 *   - POSITIONAL: represents a positional argument (input file)
 * - Constraints are checked generically based on option types, not hardcoded logic
 * - This design makes it trivial to add new options without modifying constraint logic
 */

#include "driver/command_line_argument_parser.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/common.h"
#include "driver/initializer.h"
#include "middle-end/optimization_pass.h"

/// @brief Enumerates the categories of command line options
/// @details These categories define compatibility rules - what options can be combined
typedef enum {
    /// Singleton options: mutually exclusive with all other options (e.g., help, version)
    OPTION_TYPE_SINGLETON,
    /// Modifier options: can be combined with other modifiers and positional args
    OPTION_TYPE_MODIFIER,
    /// Positional argument: the input file (only one allowed)
    OPTION_TYPE_POSITIONAL
} option_category_t;

/// @brief Models the configuration for a single command line option
/// @details The type field defines all constraints - no hardcoded logic needed elsewhere
typedef struct {
    /// @brief The short representation (e.g., "-h")
    char const * shortRepresentation;
    /// @brief The long representation (e.g., "--help")
    char const * longRepresentation;
    /// @brief Option category that defines what this can be combined with
    option_category_t category;
} command_line_option_config_t;

/// @brief Configuration for all available options
/// @details The only constraint information needed is in the category field.
/// Compatibility rules are defined once generically and apply universally.
static const command_line_option_config_t option_configs[] = {
    [OPTION_HELP] = {
        .shortRepresentation = "-h",
        .longRepresentation = "--help",
        .category = OPTION_TYPE_SINGLETON
    },
    [OPTION_VERSION] = {
        .shortRepresentation = "-v",
        .longRepresentation = "--version",
        .category = OPTION_TYPE_SINGLETON
    },
    [OPTION_COMPILE] = {
        .shortRepresentation = "-c",
        .longRepresentation = "--compile",
        .category = OPTION_TYPE_MODIFIER
    },
    [OPTION_OPTIMIZATION_LEVEL] = {
        .shortRepresentation = NULL,  // Handled specially: -ON or --optimize=N or --optimize N
        .longRepresentation = NULL,
        .category = OPTION_TYPE_MODIFIER
    }
};

// Helper function declarations
static void command_line_argument_parser_error(char const *, ...);
static inline bool command_line_argument_parser_is_option(char const *);
static bool command_line_argument_parser_parse_optimization_level(char const *, uint32_t *, bool *);
static bool command_line_argument_parser_try_parse_option(char const *, command_line_option_type *);
static bool command_line_argument_parser_can_add_option(command_line_config_t const *, option_category_t);

/// @brief Checks if a new option can be combined with the current parse state
/// @details Uses option categories to determine compatibility:
/// - At most one SINGLETON option
/// - Multiple MODIFIER options allowed
/// - At most one POSITIONAL argument
/// - SINGLETONs cannot combine with any other option (modifiers or positional)
/// @return True if the option can be added, false if it violates constraints
static bool command_line_argument_parser_can_add_option(
    command_line_config_t const * config,
    option_category_t new_option_category
) {
    // Check if we already have a singleton
    if (config->help || config->version) {
        // Cannot add anything to a singleton
        return false;
    }

    // If the new option is a singleton, check if we already have modifiers or positional
    if (new_option_category == OPTION_TYPE_SINGLETON) {
        return (config->compile == false && config->optimizationLevel == UINT32_MAX &&
                config->inputFile == NULL);
    }

    // MODIFIER options can combine with other modifiers and positional args
    // POSITIONAL is the input file - already checked ("only one input file")
    return true;
}

/// @brief Parses command line arguments and populates the configuration structure
/// @details All constraint validation is type-based and defined once, making this
/// function generic and easy to extend. Adding new options requires only:
/// 1. Adding enum value to command_line_option_type
/// 2. Adding config entry with appropriate category
/// No changes to this function needed.
command_line_config_t command_line_argument_parser_parse(int argc, char const ** argv) {
    command_line_config_t config = {
        .help = false,
        .version = false,
        .compile = false,
        .inputFile = NULL,
        .optimizationLevel = UINT32_MAX
    };

    bool expectOptimizationLevelValue = false;

    for (int i = 1; i < argc; i++) {
        // Handle pending optimization level value (from --optimize without =)
        if (expectOptimizationLevelValue) {
            if (!command_line_argument_parser_parse_optimization_level(argv[i], &config.optimizationLevel,
                                                                       &expectOptimizationLevelValue)) {
                command_line_argument_parser_error("Missing or invalid optimization level after --optimize");
            }
            continue;
        }

        // Try to parse optimization level (may set expectOptimizationLevelValue to true)
        if (command_line_argument_parser_parse_optimization_level(argv[i], &config.optimizationLevel,
                                                                  &expectOptimizationLevelValue)) {
            if (!command_line_argument_parser_can_add_option(&config, OPTION_TYPE_MODIFIER)) {
                command_line_argument_parser_error("Optimization level cannot be combined with help or version");
            }
            continue;
        }

        // Check if this is an option (starts with -)
        if (command_line_argument_parser_is_option(argv[i])) {
            command_line_option_type option;
            if (!command_line_argument_parser_try_parse_option(argv[i], &option)) {
                command_line_argument_parser_error("Unknown option: %s", argv[i]);
            }

            option_category_t category = option_configs[option].category;

            // Validate constraint based on option category
            if (!command_line_argument_parser_can_add_option(&config, category)) {
                if (category == OPTION_TYPE_SINGLETON) {
                    command_line_argument_parser_error(
                        "Help and version options cannot be combined with other options");
                } else if (category == OPTION_TYPE_MODIFIER) {
                    command_line_argument_parser_error(
                        "Options cannot be combined with help or version");
                }
            }

            // Apply the option to the configuration
            if (option == OPTION_HELP) {
                config.help = true;
            } else if (option == OPTION_VERSION) {
                config.version = true;
            } else if (option == OPTION_COMPILE) {
                config.compile = true;
            }
        } else {
            // This is a non-option argument (input file)
            if (config.inputFile != NULL) {
                command_line_argument_parser_error("Multiple input files specified");
            }

            if (!command_line_argument_parser_can_add_option(&config, OPTION_TYPE_POSITIONAL)) {
                command_line_argument_parser_error("Input file cannot be combined with help or version");
            }

            config.inputFile = argv[i];
        }
    }

    // Check for pending optimization level value
    if (expectOptimizationLevelValue) {
        command_line_argument_parser_error("Missing optimization level after --optimize");
    }

    return config;
}

/// @brief Emits an error message and exits with command-line usage error code
/// @param format The format string for the error message
/// @param ... The format arguments
static void command_line_argument_parser_error(char const * format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
    exit(EXIT_CODE_COMMAND_LINE_USAGE_ERROR);
}

/// @brief Determines if an argument is an option (starts with '-')
/// @param argument The argument to check
/// @return True if the argument starts with '-'
static inline bool command_line_argument_parser_is_option(char const * argument) {
    return argument[0] == '-';
}

/// @brief Attempts to parse an option and returns its type
/// @param option The option string to parse (e.g., "-h" or "--help")
/// @param out_option Pointer to store the parsed option type
/// @return True if the option was recognized, false otherwise
static bool command_line_argument_parser_try_parse_option(char const * option,
                                                          command_line_option_type * out_option) {
    // Compare against all known options
    for (size_t i = 0; i < sizeof(option_configs) / sizeof(command_line_option_config_t); i++) {
        if (option_configs[i].shortRepresentation != NULL &&
            !strcmp(option_configs[i].shortRepresentation, option)) {
            *out_option = (command_line_option_type)i;
            return true;
        }
        if (option_configs[i].longRepresentation != NULL && !strcmp(option_configs[i].longRepresentation, option)) {
            *out_option = (command_line_option_type)i;
            return true;
        }
    }
    return false;
}

/// @brief Parses an optimization level argument
/// @details Supports three formats:
/// - Short form: -O0, -O1, -O2, -O3
/// - Long form with equals: --optimize=0, --optimize=1, --optimize=2, --optimize=3
/// - Long form with space: --optimize followed by 0, 1, 2, or 3
/// @param option The argument to parse
/// @param out_level Pointer to store the parsed optimization level (0-3)
/// @param out_expect_value Set to true if --optimize was found without a value
/// @return True if this was an optimization level argument, false otherwise
static bool command_line_argument_parser_parse_optimization_level(char const * option,
                                                                  uint32_t * out_level,
                                                                  bool * out_expect_value) {
    // Handle bare -O or --optimize (expects next arg to contain the level)
    if (!strcmp(option, "-O") || !strcmp(option, "--optimize")) {
        *out_expect_value = true;
        return true;
    }

    // Handle -ON format (-O0, -O1, -O2, -O3)
    if (!strncmp(option, "-O", 2)) {
        if (strlen(option) != 3 || option[2] < '0' || option[2] > '3') {
            command_line_argument_parser_error("Invalid optimization level: %s (expected: -O0 to -O3)", option);
        }
        *out_level = option[2] - '0';
        *out_expect_value = false;
        return true;
    }

    // Handle --optimize=N format (--optimize=0, --optimize=1, etc.)
    if (!strncmp(option, "--optimize=", 11)) {
        if (strlen(option) != 12 || option[11] < '0' || option[11] > '3') {
            command_line_argument_parser_error("Invalid optimization level: %s (expected: --optimize=0 to --optimize=3)",
                                               option);
        }
        *out_level = option[11] - '0';
        *out_expect_value = false;
        return true;
    }

    // Handle space-separated value: the next argument after --optimize should be 0-3
    if (*out_expect_value) {
        if (strlen(option) != 1 || option[0] < '0' || option[0] > '3') {
            // Invalid value for optimization level
            return false;
        }
        *out_level = option[0] - '0';
        *out_expect_value = false;
        return true;
    }

    return false;
}