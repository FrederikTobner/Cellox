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
 * @file module_parser.h
 * @brief Parser for Cellox module source text.
 * @details Extracts import/export declarations from source, strips the
 * `export` keyword, and collects the information needed by the module loader.
 * This module has no dependency on the module-graph state kept by module_loader.
 */

#ifndef CELLOX_MODULE_PARSER_H_
#define CELLOX_MODULE_PARSER_H_

#include <stddef.h>

#include "base/common.h"

/// @brief Describes a single `import` statement parsed from a source file.
typedef struct {
    /// Path string extracted from the import statement.
    char * path;
    /// Array of symbol names from a named import (`{ A, B } from "…"`).
    char ** importedNames;
    /// Number of entries in `importedNames`. Zero for bare-path imports.
    size_t importedNameCount;
} import_spec_t;

/// @brief Growing list of export names declared in a module.
typedef struct {
    char ** names;
    size_t count;
    size_t capacity;
} export_list_t;

/// @brief Growable character buffer used for source assembly.
typedef struct {
    char * chars;
    size_t length;
    size_t capacity;
} source_buffer_t;

/// @brief Appends `length` bytes from `text` to `buffer`, growing it as needed.
/// @param buffer The destination buffer.
/// @param text   The data to append.
/// @param length Number of bytes to append.
void module_parser_append_source(source_buffer_t * buffer, char const * text, size_t length);

/// @brief Parses `source`, collects exports/imports, and returns a transformed source.
/// @details The `export` keyword is stripped from each export declaration so
///          the resulting source can be compiled without special handling.
///          On success the caller owns `*transformedSource` and must free it.
///          On failure `*imports` and `*importCount` are reset to NULL/0, and
///          any partially-built exports remain in `exports` for the caller to
///          free with module_parser_export_list_free().
/// @param source           NUL-terminated source text to parse.
/// @param exports          Out-parameter; receives the collected export names.
/// @param imports          Out-parameter; receives a malloc'd array of import specs.
/// @param importCount      Out-parameter; receives the number of import specs.
/// @param transformedSource Out-parameter; receives the transformed, malloc'd source.
/// @return true on success, false on any parse or allocation error.
bool module_parser_parse(char const * source, export_list_t * exports, import_spec_t ** imports, size_t * importCount,
                         char ** transformedSource);

/// @brief Frees the contents of an `import_spec_t` array allocated by module_parser_parse().
/// @param imports    The array to free. May be NULL.
/// @param importCount Number of elements in `imports`.
void module_parser_cleanup_imports(import_spec_t * imports, size_t importCount);

/// @brief Frees all export names stored in `list` and resets its fields to zero.
/// @param list The export list to free. Must not be NULL.
void module_parser_export_list_free(export_list_t * list);

#endif // CELLOX_MODULE_PARSER_H_
