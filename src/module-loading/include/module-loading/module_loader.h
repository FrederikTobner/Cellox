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

#endif
