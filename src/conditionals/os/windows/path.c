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

#include "clx_os/path.h"

#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * clx_os_path_canonicalize(char const * path) {
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
}

bool clx_os_path_is_absolute(char const * path) {
    if (!path || !path[0]) {
        return false;
    }

    return path[0] == '/' || path[0] == '\\' || (strlen(path) > 1 && path[1] == ':');
}

char const * clx_os_path_find_last_separator(char const * path) {
    if (!path) {
        return NULL;
    }

    char const * slashSeparator = strrchr(path, '/');
    char const * backslashSeparator = strrchr(path, '\\');
    if (!slashSeparator) {
        return backslashSeparator;
    }
    if (!backslashSeparator) {
        return slashSeparator;
    }

    return backslashSeparator > slashSeparator ? backslashSeparator : slashSeparator;
}

char clx_os_path_separator(void) {
    return '\\';
}

char * clx_os_path_executable_dir(void) {
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, buf, sizeof(buf));
    if (len == 0 || len == sizeof(buf)) {
        return NULL;
    }
    char * lastSlash = strrchr(buf, '\\');
    char * lastFwdSlash = strrchr(buf, '/');
    char * last = lastSlash > lastFwdSlash ? lastSlash : lastFwdSlash;
    if (!last) {
        return strdup(".");
    }
    size_t dirLen = (size_t)(last - buf);
    char * dir = malloc(dirLen + 1);
    if (!dir) {
        return NULL;
    }
    memcpy(dir, buf, dirLen);
    dir[dirLen] = '\0';
    return dir;
}