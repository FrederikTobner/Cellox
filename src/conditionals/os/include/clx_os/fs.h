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

#ifndef CELLOX_OS_FS_H_
#define CELLOX_OS_FS_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool clx_os_fs_ensure_directory(char const * path);
/// Returns true if the given path exists (file or directory).
bool clx_os_fs_path_exists(char const * path);

#ifdef __cplusplus
}
#endif

#endif // CELLOX_OS_FS_H_