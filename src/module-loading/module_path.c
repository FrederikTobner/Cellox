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
 */

#include "module_path.h"

#include <stdlib.h>
#include <string.h>

#include "clx_os/path.h"
#include "string_utils.h"

char * module_path_canonicalize(char const * path) {
    return clx_os_path_canonicalize(path);
}

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

char * module_path_resolve_import(char const * importerPath, char const * importPath) {
    if (!importPath || !importPath[0]) {
        return NULL;
    }

    bool isAbsolute = clx_os_path_is_absolute(importPath);

    if (isAbsolute) {
        return string_utils_strdup(importPath);
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
