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

#ifndef CELLOX_OS_PATH_H_
#define CELLOX_OS_PATH_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

char * clx_os_path_canonicalize(char const * path);
bool clx_os_path_is_absolute(char const * path);
char const * clx_os_path_find_last_separator(char const * path);
char clx_os_path_separator(void);
/// Returns a heap-allocated string containing the directory of the running executable,
/// or NULL on failure. The caller does not need to free the result; it is cached internally.
char * clx_os_path_executable_dir(void);

#ifdef __cplusplus
}
#endif

#endif // CELLOX_OS_PATH_H_