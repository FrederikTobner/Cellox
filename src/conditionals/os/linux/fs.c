/****************************************************************************
cu// ============================================================================
// Global Pipeline State
// ============================================================================

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

#include "clx_os/fs.h"

#include <errno.h>
#include <sys/stat.h>

bool clx_os_fs_ensure_directory(char const * path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode);
    }

    if (errno != ENOENT) {
        return false;
    }

    if (mkdir(path, 0700) != 0) {
        return false;
    }

    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}