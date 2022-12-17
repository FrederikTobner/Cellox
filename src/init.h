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
 * @file init.h
 * @brief Header file that is used to initialize the interpreter
 * @details The interpreter can be initialized to run from a file or as repl.
 */

#ifndef CELLOX_INIT_H_
#define CELLOX_INIT_H_

// Init.h is included in the test-suite that is written in c++ using the google-test framework
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/// Message that explains the usage of the cellox interpreter
#define CELLOX_USAGE_MESSAGE ("Usage: Cellox ((-h|--help|-v|--version) | ((-scf | --store-chunk-file) |(-rcf |--read-chunk-file)) [path]\n")

/** @brief Run with repl
* @details  
* <ol>
* <li> Read the user input   -   (R)</li>
* <li> Evaluate your code    -   (E)</li>
* <li> Print any results     -   (P)</li>
* <li> Loop back to step 1   -   (L)</li>
* </ol>
*/
void init_repl();

/// @brief Reads a lox program from a file and executes the program
/// @param path The path of the lox program
/// @param compile boolean value that determines whether the compiled program is stored as a chunk file
void init_run_from_file(char const * path, bool compile);

/// Shows the help of the cellox interpreter
void init_show_help();

/// Shows the version of the cellox interpreter
void init_show_version();

#ifdef __cplusplus
}
#endif

#endif
