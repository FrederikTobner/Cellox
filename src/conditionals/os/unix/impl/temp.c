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

#include "clx_os/temp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char * clx_os_temp_make_path(char const * prefix, char const * suffix) {
    char const * tempDirectory = getenv("TMPDIR");
    if (!tempDirectory || !tempDirectory[0]) {
        tempDirectory = "/tmp";
    }

    if (!prefix) {
        prefix = "cellox_";
    }
    if (!suffix) {
        suffix = "";
    }

    size_t suffixLength = strlen(suffix);
    size_t pathLength = strlen(tempDirectory) + 1 + strlen(prefix) + 6 + suffixLength + 1;
    char * path = malloc(pathLength);
    if (!path) {
        return NULL;
    }

    snprintf(path, pathLength, "%s/%sXXXXXX%s", tempDirectory, prefix, suffix);
    int fd = mkstemps(path, (int)suffixLength);
    if (fd == -1) {
        free(path);
        return NULL;
    }

    close(fd);
    remove(path);
    return path;
}
