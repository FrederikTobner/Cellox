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
 * @brief File containing main entry point of the interpreter.
 */

#include "command_line_argument_parser.h"
#include "common.h"

/// @brief Main entry point of the cellox interpreter
/// @param argc The amount of arguments that were specified by the user
/// @param argv The arguments that were specified by the user
/// @return 0 if no error occurs
int main(int argc, char const ** argv)
{    
    command_line_argument_parser_parse(argc, argv);
    return EXIT_CODE_OK;
}
