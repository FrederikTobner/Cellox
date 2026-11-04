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
 * @file pass_utils.h
 * @brief Helpers shared by the optimization passes that rewrite bytecode.
 * @details Peephole patterns are only valid if no jump enters the middle of them, and removing bytes from a chunk
 * requires the offsets of the jumps that span the removed region to be adjusted.
 */

#ifndef CELLOX_PASS_UTILS_H_
#define CELLOX_PASS_UTILS_H_

#include <stdbool.h>
#include <stdint.h>

#include "byte-code/chunk.h"

/// @brief Determines whether a jump or loop instruction of the chunk targets the given offset
/// @param chunk The chunk that is inspected
/// @param offset The bytecode offset that is checked
/// @return True if at least one jump instruction targets the offset
bool pass_utils_is_jump_target(chunk_t * chunk, uint32_t offset);

/// @brief Determines whether a jump or loop instruction targets an offset in the range [first, last]
/// @param chunk The chunk that is inspected
/// @param first The first offset of the range
/// @param last The last offset of the range
/// @return True if at least one jump instruction targets an offset inside the range
bool pass_utils_has_jump_target_in_range(chunk_t * chunk, uint32_t first, uint32_t last);

/// @brief Removes bytes from the chunk and adjusts the jumps that span the removed region
/// @details No jump may target an offset strictly inside the removed region, use
/// pass_utils_has_jump_target_in_range to ensure that.
/// @param chunk The chunk the bytes are removed from
/// @param start The offset of the first byte that is removed
/// @param amount The number of bytes that are removed
void pass_utils_remove_bytecode(chunk_t * chunk, uint32_t start, uint32_t amount);

#endif
