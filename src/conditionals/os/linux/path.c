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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char * clx_os_path_canonicalize(char const * path) {
    return realpath(path, NULL);
}

bool clx_os_path_is_absolute(char const * path) {
    return path && path[0] == '/';
}

char const * clx_os_path_find_last_separator(char const * path) {
    if (!path) {
        return NULL;
    }

    return strrchr(path, '/');
}

char clx_os_path_separator(void) {
    return '/';
}

char * clx_os_path_executable_dir(void) {
    char buf[4096];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len <= 0) {
        return NULL;
    }
    buf[len] = '\0';
    char * last = strrchr(buf, '/');
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