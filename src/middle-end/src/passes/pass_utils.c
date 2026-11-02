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
 * @file pass_utils.c
 * @brief Helpers shared by the optimization passes that rewrite bytecode.
 */

#include "pass_utils.h"

static bool pass_utils_decode_jump(chunk_t * chunk, uint32_t offset, int64_t * target);

bool pass_utils_is_jump_target(chunk_t * chunk, uint32_t offset) {
    return pass_utils_has_jump_target_in_range(chunk, offset, offset);
}

bool pass_utils_has_jump_target_in_range(chunk_t * chunk, uint32_t first, uint32_t last) {
    for (uint32_t i = 0; i < chunk->byteCodeCount; i += chunk_determine_opcode_size_by_index(chunk, i)) {
        int64_t target;
        if (pass_utils_decode_jump(chunk, i, &target) && target >= (int64_t)first && target <= (int64_t)last) {
            return true;
        }
    }
    return false;
}

void pass_utils_remove_bytecode(chunk_t * chunk, uint32_t start, uint32_t amount) {
    uint32_t end = start + amount;
    for (uint32_t i = 0; i < chunk->byteCodeCount; i += chunk_determine_opcode_size_by_index(chunk, i)) {
        int64_t target;
        if (!pass_utils_decode_jump(chunk, i, &target)) {
            continue;
        }
        uint16_t distance = (uint16_t)(((uint16_t)chunk->code[i + 1] << 8) | (uint16_t)chunk->code[i + 2]);
        bool spansRemoval;
        if (chunk->code[i] == OP_LOOP) {
            spansRemoval = i >= end && target <= (int64_t)start;
        } else {
            spansRemoval = i < start && target >= (int64_t)end;
        }
        if (spansRemoval) {
            distance = (uint16_t)(distance - amount);
            chunk->code[i + 1] = (uint8_t)((distance >> 8) & 0xFFu);
            chunk->code[i + 2] = (uint8_t)(distance & 0xFFu);
        }
    }
    chunk_remove_bytecode(chunk, start, amount);
}

static bool pass_utils_decode_jump(chunk_t * chunk, uint32_t offset, int64_t * target) {
    uint8_t opcode = chunk->code[offset];
    if ((opcode != OP_JUMP && opcode != OP_JUMP_IF_FALSE && opcode != OP_LOOP) || offset + 2 >= chunk->byteCodeCount) {
        return false;
    }
    uint16_t distance = (uint16_t)(((uint16_t)chunk->code[offset + 1] << 8) | (uint16_t)chunk->code[offset + 2]);
    *target = opcode == OP_LOOP ? (int64_t)offset + 3 - distance : (int64_t)offset + 3 + distance;
    return true;
}
