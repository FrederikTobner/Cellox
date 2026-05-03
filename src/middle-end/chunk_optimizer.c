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
 * @file chunk_optimizer.c
 * @brief File containing the implementation of functionality regarding the optimization of cellox chunks.
 */

#include "chunk_optimizer.h"
#include "optimization_pass.h"

void chunk_optimizer_optimize_chunk(chunk_t * chunk) {
    // Delegate to the optimization pass framework
    // This runs the entire pipeline on the chunk
    optimization_pipeline_run_chunk(chunk, "unknown_chunk");
}
