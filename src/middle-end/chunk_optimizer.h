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
 * @file chunk_optimizer.h
 * @brief Header file containing the declarations of functionality regarding the optimization of cellox chunks.
 */

#ifndef CELLOX_CHUNK_OPTIMIZER_H_
#define CELLOX_CHUNK_OPTIMIZER_H_

#include "../byte-code/chunk.h"

/// @brief Optimizes the chunk by using different compiler optimization techniques
/// @param chunk The chunk that is optimized
/// @details At the moment only constant folding is used
void chunk_optimizer_optimize_chunk(chunk_t * chunk);

#endif