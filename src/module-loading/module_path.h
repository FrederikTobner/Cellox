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
 * @file module_path.h
 * @brief Filesystem path utilities used by the module loader.
 * @details Provides portable helpers for resolving, canonicalising, and
 * joining file-system paths.  All functions are pure path manipulations with
 * no dependency on module-graph state.
 */

#ifndef CELLOX_MODULE_PATH_H_
#define CELLOX_MODULE_PATH_H_

/// Platform path separator character
#if defined(_WIN32)
#    define MODULE_PATH_SEPARATOR '\\'
#else
#    define MODULE_PATH_SEPARATOR '/'
#endif

/// @brief Returns the canonical (absolute, symlink-resolved) path for `path`.
/// @param path The raw path to canonicalise.
/// @return A malloc'd canonical path string the caller must free, or NULL if
///         the file does not exist or the path cannot be resolved.
char * module_path_canonicalize(char const * path);

/// @brief Joins two path segments with the platform path separator.
/// @param left  The directory part (may or may not end with a separator).
/// @param right The file/subdirectory part to append.
/// @return A malloc'd joined path that the caller must free, or NULL on OOM.
char * module_path_join(char const * left, char const * right);

/// @brief Resolves an import path relative to the importer's directory.
/// @param importerPath Canonical path of the importing file.
/// @param importPath   The path string from the import statement.
/// @return A malloc'd resolved path the caller must free, or NULL on failure.
///         Absolute import paths are duplicated as-is.
char * module_path_resolve_import(char const * importerPath, char const * importPath);

#endif /* CELLOX_MODULE_PATH_H_ */
