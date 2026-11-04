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
 * @file branch_predication.c
 * @brief Branch predication optimization pass
 */

#include "middle-end/optimization_pass.h"

#include "pass_utils.h"

static bool is_statically_falsey(uint8_t opcode);
static bool is_statically_truthy(uint8_t opcode);
static void write_u16_be(uint8_t * data, uint16_t value);

pass_result_t pass_branch_predication(chunk_t * chunk) {
    pass_result_t result = {
        .pass_name = "branch_predication",
        .modified = false,
        .instructions_removed = 0,
        .constants_folded = 0,
        .branches_eliminated = 0,
    };

    // Instructions are visited by their boundaries, operand bytes must not be mistaken for opcodes
    uint32_t size;
    for (uint32_t i = 0; i + 3 < chunk->byteCodeCount; i += size) {
        size = chunk_determine_opcode_size_by_index(chunk, i);
        uint8_t condition_op = chunk->code[i];
        if (!is_statically_truthy(condition_op) && !is_statically_falsey(condition_op)) {
            continue;
        }

        // Pattern: OP_TRUE/OP_FALSE/OP_NULL, OP_JUMP_IF_FALSE, hi, lo
        if (chunk->code[i + 1] != OP_JUMP_IF_FALSE) {
            continue;
        }

        // A jump that lands on the conditional jump brings a condition that is not statically known
        if (pass_utils_is_jump_target(chunk, i + 1)) {
            continue;
        }

        // Rewrite to unconditional jump, preserving stack behavior:
        // - truthy condition: jump distance 0 (acts as no-op branch)
        // - falsey condition: keep original jump distance
        chunk->code[i + 1] = OP_JUMP;
        if (is_statically_truthy(condition_op)) {
            write_u16_be(&chunk->code[i + 2], 0);
        }

        result.modified = true;
        result.branches_eliminated++;
    }

    return result;
}

static bool is_statically_falsey(uint8_t opcode) {
    return opcode == OP_FALSE || opcode == OP_NULL;
}

static bool is_statically_truthy(uint8_t opcode) {
    return opcode == OP_TRUE;
}

static void write_u16_be(uint8_t * data, uint16_t value) {
    data[0] = (uint8_t)((value >> 8) & 0xFFu);
    data[1] = (uint8_t)(value & 0xFFu);
}
