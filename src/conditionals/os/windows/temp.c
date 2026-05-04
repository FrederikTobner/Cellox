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
#include <windows.h>

char * clx_os_temp_make_path(char const * prefix, char const * suffix) {
    char tempDirectory[MAX_PATH];
    DWORD directoryLength = GetTempPathA(MAX_PATH, tempDirectory);
    if (directoryLength == 0 || directoryLength >= MAX_PATH) {
        return NULL;
    }

    char prefixBuffer[4] = "clx";
    if (prefix && prefix[0] && prefix[1] && prefix[2]) {
        prefixBuffer[0] = prefix[0];
        prefixBuffer[1] = prefix[1];
        prefixBuffer[2] = prefix[2];
    }

    char tempFile[MAX_PATH];
    UINT result = GetTempFileNameA(tempDirectory, prefixBuffer, 0, tempFile);
    if (result == 0) {
        return NULL;
    }

    remove(tempFile);

    if (!suffix) {
        suffix = "";
    }

    size_t tempFileLength = strlen(tempFile);
    size_t suffixLength = strlen(suffix);
    char * path = malloc(tempFileLength + suffixLength + 1);
    if (!path) {
        return NULL;
    }

    memcpy(path, tempFile, tempFileLength);
    memcpy(path + tempFileLength, suffix, suffixLength);
    path[tempFileLength + suffixLength] = '\0';
    return path;
}