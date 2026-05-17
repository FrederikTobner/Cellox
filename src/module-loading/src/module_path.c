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
 * @file module_path.c
 * @brief Filesystem path utilities used by the module loader.
 * @details Keeps path resolution policy in one place so module loading can
 * stay focused on graph traversal and import validation.
 */

#include <module-loading/internal/module_path.h>

#include <stdlib.h>
#include <string.h>

#include "clx_os/path.h"
#include "utils/string_utils.h"

/**
 * @brief Canonicalises a path using the active OS abstraction layer.
 * @param path Raw path to canonicalise.
 * @return A newly allocated canonical path, or NULL on failure.
 */
char * module_path_canonicalize(char const * path) {
    return clx_os_path_canonicalize(path);
}

/**
 * @brief Joins two path segments using the platform separator.
 * @param left Left-hand path segment.
 * @param right Right-hand path segment.
 * @return A newly allocated joined path, or NULL on allocation failure.
 */
char * module_path_join(char const * left, char const * right) {
    char separator = clx_os_path_separator();
    size_t leftLength = strlen(left);
    size_t rightLength = strlen(right);
    size_t hasSeparator = leftLength && (left[leftLength - 1] == '/' || left[leftLength - 1] == '\\');
    size_t totalLength = leftLength + rightLength + (hasSeparator ? 0 : 1) + 1;
    char * joined = malloc(totalLength);
    if (!joined) {
        return NULL;
    }

    memcpy(joined, left, leftLength);
    size_t offset = leftLength;
    if (!hasSeparator) {
        joined[offset++] = separator;
    }
    memcpy(joined + offset, right, rightLength);
    joined[offset + rightLength] = '\0';
    return joined;
}

/**
 * @brief Resolves an import path against an importer or stdlib root.
 * @details Absolute paths are returned unchanged, bare imports are resolved
 * against `stdlibPath`, and relative imports are resolved from the importer's
 * containing directory.
 * @param importerPath Canonical path of the importing module.
 * @param importPath Raw import string from the source file.
 * @param stdlibPath Standard-library root used for bare imports, or NULL.
 * @return A newly allocated resolved path, or NULL on failure.
 */
char * module_path_resolve_import(char const * importerPath, char const * importPath, char const * stdlibPath) {
    if (!importPath || !importPath[0]) {
        return NULL;
    }

    bool isAbsolute = clx_os_path_is_absolute(importPath);

    if (isAbsolute) {
        return string_utils_strdup(importPath);
    }

    /* A bare import does not start with '.', '/', or a Windows drive letter.
     * Example: "stdlib/math.clx"  →  <stdlibPath>/math.clx
     * The prefix "stdlib/" is stripped so users write `from "stdlib/math.clx"` and
     * the resolved path becomes `<stdlibPath>/math.clx`. */
    bool isBare = importPath[0] != '.' && importPath[0] != '/' && importPath[0] != '\\' &&
                  !(strlen(importPath) > 1 && importPath[1] == ':');
    if (isBare && stdlibPath) {
        /* Strip an optional leading "stdlib/" prefix */
        char const * relativePart = importPath;
        if (strncmp(importPath, "stdlib/", 7) == 0) {
            relativePart = importPath + 7;
        } else if (strncmp(importPath, "stdlib\\", 7) == 0) {
            relativePart = importPath + 7;
        }
        return module_path_join(stdlibPath, relativePart);
    }

    char const * lastSeparator = clx_os_path_find_last_separator(importerPath);

    if (!lastSeparator) {
        return string_utils_strdup(importPath);
    }

    size_t directoryLength = (size_t)(lastSeparator - importerPath);
    char * directory = string_utils_substr(importerPath, directoryLength);
    if (!directory) {
        return NULL;
    }

    char * resolvedPath = module_path_join(directory, importPath);
    free(directory);
    return resolvedPath;
}
