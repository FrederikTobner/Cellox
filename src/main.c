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
 * @file main.c
 * @brief File containing main entry point of the compiler.
 */

#include <stdlib.h>

#include "driver/command_line_argument_parser.h"
#include "base/common.h"
#include "driver/initializer.h"
#include "middle-end/optimization_pass.h"

/// @brief Main entry point of the cellox compiler
/// @param argc The amount of arguments that were specified by the user
/// @param argv The arguments that were specified by the user
/// @return 0 if no error occurs; otherwise an appropriate error code
int main(int argc, char const ** argv) {
    // Parse command line arguments into a configuration structure
    command_line_config_t config = command_line_argument_parser_parse(argc, argv);

    // Apply optimization level if specified
    if (config.optimizationLevel != UINT32_MAX) {
        optimization_set_level(config.optimizationLevel);
    }

    // Dispatch based on configuration
    if (config.help) {
        initializer_show_help();
    } else if (config.version) {
        initializer_show_version();
    } else if (config.inputFile != NULL) {
        // Either compile mode or normal mode (compile flag is optional)
        initializer_run_from_file(config.inputFile, config.compile);
    } else {
        // No input file and no action option: interactive mode (REPL)
        initializer_run_as_repl();
    }

    return EXIT_SUCCESS;
}
