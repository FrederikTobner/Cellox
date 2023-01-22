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
 * See the <"https://www.gnu.org/licenses/gpl-3.0.html">GNU General Public  *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file compiler.h
 * @brief Header file of the compiler
 */

#ifndef CELLOX_COMPILER_H_
#define CELLOX_COMPILER_H_

#include "../frontend/lexer.h"
#include "../backend/object.h"

/// @brief Compiles a cellox program.
/// @param code The cellox program that is compiled
/// @return A obect_function_t that that stores all the instructions that of the cellox program
/// @details Converts the textual representation of a cellox program in a cellox chunk.
/// The source code is for that purposed scanned, parsed and then converted to a intermediate representation
object_function_t * compiler_compile(char const * code);

/// @brief Marks the compiler roots.
/// @details These are all the the objects that can be directly accessed by the virtualMachine and not through a reference in some other object.
void compiler_mark_roots();

#endif
