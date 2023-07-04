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
 * @file initializer.h
 * @brief Header file that is used to initialize the compiler
 * @details The compiler can be initialized to run from a file or as repl.
 */

#ifndef CELLOX_INITIALIZER_H_
#define CELLOX_INITIALIZER_H_

// This file is included in the test-suite that is written in c++ using the google-test framework
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/// Message that explains the usage of the cellox compiler
#define CELLOX_USAGE_MESSAGE ("Usage: Cellox ((-h|--help|-v|--version) | ((-c | --compile) [path])\n")

/** @brief Run with repl
 * @details
 * <ol>
 * <li> Read the user input   -   (R)</li>
 * <li> Evaluate your code    -   (E)</li>
 * <li> Print any results     -   (P)</li>
 * <li> Loop back to step 1   -   (L)</li>
 * </ol>
 */
void initializer_run_as_repl(void);

/// @brief Reads a lox program from a file and executes the program
/// @param path The path of the lox program
/// @param compile boolean value that determines whether the compiled program is stored as a chunk file
void initializer_run_from_file(char const * path, bool compile);

/// Shows the help of the cellox compiler
void initializer_show_help(void);

/// Shows the version of the cellox compiler
void initializer_show_version(void);

#ifdef __cplusplus
}
#endif

#endif
