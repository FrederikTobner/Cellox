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

#ifndef CELLOX_OS_TIME_H_
#define CELLOX_OS_TIME_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t clx_os_time_now_ns(void);

#ifdef __cplusplus
}
#endif

#endif // CELLOX_OS_TIME_H_
