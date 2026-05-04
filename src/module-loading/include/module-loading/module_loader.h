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
 * @file module_loader.h
 * @brief Header file containing declarations regarding module loading.
 */

#ifndef CELLOX_MODULE_LOADER_H_
#define CELLOX_MODULE_LOADER_H_

/// @brief Loads a module graph starting from the specified entry path.
/// @details Resolves and validates import and export declarations and returns a single source text
/// that can be compiled by the existing compiler frontend.
/// @param entryPath Path to the entry module.
/// @return Heap-allocated source text or NULL if loading failed.
char * module_loader_load_program(char const * entryPath);

/// @brief Overrides the standard library search path used to resolve bare imports.
/// @details A "bare" import is one that does not start with '.', '/', or a drive letter.
///          For example, `import { abs } from "stdlib/math.clx"` will be resolved as
///          `<stdlibPath>/math.clx` when stdlibPath is set to the directory containing math.clx.
///          If not called, the path compiled in via CLX_STDLIB_PATH is used.
///          Passing NULL resets to the compiled-in default.
/// @param path The directory that contains the stdlib modules, or NULL to reset.
void module_loader_set_stdlib_path(char const * path);

#endif
