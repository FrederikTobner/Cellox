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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#    include <direct.h>
#else
#    include <limits.h>
#    include <unistd.h>
#endif

#include "string_utils.h"

char * module_path_canonicalize(char const * path) {
#if defined(_WIN32)
    char * resolved = _fullpath(NULL, path, 0);
    if (!resolved) {
        return NULL;
    }
    /* _fullpath does not check whether the file exists; verify before returning */
    FILE * check = fopen(resolved, "rb");
    if (!check) {
        free(resolved);
        return NULL;
    }
    fclose(check);
    return resolved;
#else
    return realpath(path, NULL);
#endif
}

char * module_path_join(char const * left, char const * right) {
    size_t leftLength = strlen(left);
    size_t rightLength = strlen(right);
    size_t hasSeparator = leftLength && left[leftLength - 1] == MODULE_PATH_SEPARATOR;
    size_t totalLength = leftLength + rightLength + (hasSeparator ? 0 : 1) + 1;
    char * joined = malloc(totalLength);
    if (!joined) {
        return NULL;
    }

    memcpy(joined, left, leftLength);
    size_t offset = leftLength;
    if (!hasSeparator) {
        joined[offset++] = MODULE_PATH_SEPARATOR;
    }
    memcpy(joined + offset, right, rightLength);
    joined[offset + rightLength] = '\0';
    return joined;
}

char * module_path_resolve_import(char const * importerPath, char const * importPath) {
    if (!importPath || !importPath[0]) {
        return NULL;
    }

    bool isAbsolute = importPath[0] == '/' || importPath[0] == '\\';
#if defined(_WIN32)
    if (strlen(importPath) > 1 && importPath[1] == ':') {
        isAbsolute = true;
    }
#endif

    if (isAbsolute) {
        return string_utils_strdup(importPath);
    }

    char const * lastSeparator = strrchr(importerPath, '/');
#if defined(_WIN32)
    char const * windowsSeparator = strrchr(importerPath, '\\');
    if (!lastSeparator || (windowsSeparator && windowsSeparator > lastSeparator)) {
        lastSeparator = windowsSeparator;
    }
#endif

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
