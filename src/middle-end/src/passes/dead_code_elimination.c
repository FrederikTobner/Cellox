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
 * @file dead_code_elimination.c
 * @brief Dead code detection and elimination optimization passes
 */

#include "middle-end/optimization_pass.h"

#include <stdlib.h>
#include <string.h>

#include "language-models/object.h"

#define BIT_SET(bits, idx) ((bits)[(idx) / 8] |= (1u << ((idx) % 8)))
#define BIT_GET(bits, idx) (((bits)[(idx) / 8] & (1u << ((idx) % 8))) != 0)

static void write_u16_be(uint8_t * data, uint16_t value);
static bool is_jump_opcode(uint8_t opcode);
static uint32_t opcode_size(chunk_t * chunk, uint32_t offset); 
static uint16_t read_u16_be(uint8_t const * data);
static void clear_reachability_state(chunk_t * chunk); 
static void build_dead_prefix(chunk_t * chunk, uint8_t * dead_bytes, uint32_t * prefix_removed);

static uint16_t read_u16_be(uint8_t const * data) {
    return (uint16_t)(((uint16_t)data[0] << 8) | (uint16_t)data[1]);
}

static uint32_t opcode_size(chunk_t * chunk, uint32_t offset) {
    if (offset >= chunk->byteCodeCount) {
        return 1;
    }

    switch (chunk->code[offset]) {
    case OP_CONSTANT:
    case OP_DEFINE_GLOBAL:
    case OP_GET_GLOBAL:
    case OP_GET_PROPERTY:
    case OP_GET_SUPER:
    case OP_SET_GLOBAL:
    case OP_SET_PROPERTY:
    case OP_CLASS:
    case OP_METHOD:
    case OP_ARRAY_LITERAL:
    case OP_CALL:
    case OP_GET_LOCAL:
    case OP_GET_UPVALUE:
    case OP_SET_LOCAL:
    case OP_SET_UPVALUE:
        return 2;
    case OP_JUMP:
    case OP_JUMP_IF_FALSE:
    case OP_LOOP:
    case OP_INVOKE:
    case OP_SUPER_INVOKE:
        return 3;
    case OP_CLOSURE:
        if (offset + 1 >= chunk->byteCodeCount) {
            return 1;
        }
        {
            uint8_t constant = chunk->code[offset + 1];
            if (constant < chunk->constants.count && IS_FUNCTION(chunk->constants.values[constant])) {
                object_function_t * function = AS_FUNCTION(chunk->constants.values[constant]);
                return 2 + function->upvalueCount * 2;
            }
        }
        return 2;
    default:
        return 1;
    }
}

static bool is_jump_opcode(uint8_t opcode) {
    return opcode == OP_JUMP || opcode == OP_JUMP_IF_FALSE || opcode == OP_LOOP;
}

static int32_t jump_target(chunk_t * chunk, uint32_t offset) {
    if (offset + 2 >= chunk->byteCodeCount) {
        return -1;
    }

    uint16_t jump = read_u16_be(&chunk->code[offset + 1]);
    if (chunk->code[offset] == OP_LOOP) {
        return (int32_t)offset + 3 - (int32_t)jump;
    }
    return (int32_t)offset + 3 + (int32_t)jump;
}

static void write_u16_be(uint8_t * data, uint16_t value) {
    data[0] = (uint8_t)((value >> 8) & 0xFFu);
    data[1] = (uint8_t)(value & 0xFFu);
}

static void build_dead_prefix(chunk_t * chunk, uint8_t * dead_bytes, uint32_t * prefix_removed) {
    memset(dead_bytes, 0, chunk->byteCodeCount);

    for (uint32_t offset = 0; offset < chunk->byteCodeCount;) {
        uint32_t size = opcode_size(chunk, offset);
        if (!BIT_GET(chunk->_reachable_bitset, offset)) {
            for (uint32_t i = 0; i < size && offset + i < chunk->byteCodeCount; i++) {
                dead_bytes[offset + i] = 1;
            }
        }
        offset += size;
    }

    prefix_removed[0] = 0;
    for (uint32_t i = 0; i < chunk->byteCodeCount; i++) {
        prefix_removed[i + 1] = prefix_removed[i] + dead_bytes[i];
    }
}

static void clear_reachability_state(chunk_t * chunk) {
    if (chunk->_reachable_bitset != NULL) {
        free(chunk->_reachable_bitset);
        chunk->_reachable_bitset = NULL;
        chunk->_reachable_bitset_size = 0;
    }
}

pass_result_t pass_dead_code_detection(chunk_t * chunk) {
    pass_result_t result = {
        .pass_name = "dead_code_detection",
        .modified = false,
        .instructions_removed = 0,
        .constants_folded = 0,
        .branches_eliminated = 0,
    };

    clear_reachability_state(chunk);
    if (chunk->byteCodeCount == 0) {
        return result;
    }

    size_t bitset_bytes = (chunk->byteCodeCount + 7) / 8;
    chunk->_reachable_bitset = calloc(bitset_bytes, sizeof(uint8_t));
    chunk->_reachable_bitset_size = bitset_bytes;

    uint32_t * worklist = malloc(chunk->byteCodeCount * sizeof(uint32_t));
    uint32_t worklist_count = 0;
    worklist[worklist_count++] = 0;

    while (worklist_count > 0) {
        uint32_t offset = worklist[--worklist_count];
        if (offset >= chunk->byteCodeCount || BIT_GET(chunk->_reachable_bitset, offset)) {
            continue;
        }

        BIT_SET(chunk->_reachable_bitset, offset);

        uint8_t opcode = chunk->code[offset];
        uint32_t next_offset = offset + opcode_size(chunk, offset);

        if (is_jump_opcode(opcode)) {
            int32_t target = jump_target(chunk, offset);
            if (target >= 0 && (uint32_t)target < chunk->byteCodeCount) {
                worklist[worklist_count++] = (uint32_t)target;
            }
            if (opcode == OP_JUMP_IF_FALSE && next_offset < chunk->byteCodeCount) {
                worklist[worklist_count++] = next_offset;
            }
            continue;
        }

        if (opcode != OP_RETURN && next_offset < chunk->byteCodeCount) {
            worklist[worklist_count++] = next_offset;
        }
    }

    for (uint32_t offset = 0; offset < chunk->byteCodeCount;) {
        uint32_t size = opcode_size(chunk, offset);
        if (!BIT_GET(chunk->_reachable_bitset, offset)) {
            result.instructions_removed++;
        }
        offset += size;
    }

    free(worklist);
    return result;
}

pass_result_t pass_dead_code_elimination(chunk_t * chunk) {
    pass_result_t result = {
        .pass_name = "dead_code_elimination",
        .modified = false,
        .instructions_removed = 0,
        .constants_folded = 0,
        .branches_eliminated = 0,
    };

    if (chunk->byteCodeCount == 0) {
        return result;
    }

    if (chunk->_reachable_bitset == NULL) {
        (void)pass_dead_code_detection(chunk);
    }
    if (chunk->_reachable_bitset == NULL) {
        return result;
    }

    uint8_t * dead_bytes = malloc(chunk->byteCodeCount);
    uint32_t * prefix_removed = malloc((chunk->byteCodeCount + 1) * sizeof(uint32_t));
    build_dead_prefix(chunk, dead_bytes, prefix_removed);

    // Collect dead instruction starts first, then remove from back to front.
    uint32_t * dead_offsets = malloc(chunk->byteCodeCount * sizeof(uint32_t));
    uint32_t dead_count = 0;

    for (uint32_t offset = 0; offset < chunk->byteCodeCount;) {
        uint32_t size = opcode_size(chunk, offset);
        if (!BIT_GET(chunk->_reachable_bitset, offset)) {
            dead_offsets[dead_count++] = offset;
        }
        offset += size;
    }

    // Collect jump instructions that remain reachable and must be re-targeted.
    uint32_t * jump_offsets = malloc(chunk->byteCodeCount * sizeof(uint32_t));
    uint32_t * jump_targets = malloc(chunk->byteCodeCount * sizeof(uint32_t));
    uint8_t * jump_opcodes = malloc(chunk->byteCodeCount);
    uint32_t jump_count = 0;

    for (uint32_t offset = 0; offset < chunk->byteCodeCount;) {
        uint32_t size = opcode_size(chunk, offset);
        uint8_t opcode = chunk->code[offset];
        if (BIT_GET(chunk->_reachable_bitset, offset) && is_jump_opcode(opcode)) {
            int32_t old_target = jump_target(chunk, offset);
            if (old_target >= 0 && (uint32_t)old_target <= chunk->byteCodeCount) {
                jump_offsets[jump_count] = offset;
                jump_targets[jump_count] = (uint32_t)old_target;
                jump_opcodes[jump_count] = opcode;
                jump_count++;
            }
        }
        offset += size;
    }

    for (int32_t i = (int32_t)dead_count - 1; i >= 0; i--) {
        uint32_t offset = dead_offsets[i];
        uint32_t size = opcode_size(chunk, offset);
        chunk_remove_bytecode(chunk, offset, size);
        result.instructions_removed++;
        result.modified = true;
    }

    // Re-target jump offsets after compaction.
    for (uint32_t i = 0; i < jump_count; i++) {
        uint32_t old_jump_offset = jump_offsets[i];
        uint32_t old_target = jump_targets[i];
        uint8_t opcode = jump_opcodes[i];

        uint32_t new_jump_offset = old_jump_offset - prefix_removed[old_jump_offset];
        uint32_t new_target = old_target - prefix_removed[old_target];

        if (new_jump_offset + 2 >= chunk->byteCodeCount) {
            continue;
        }

        uint32_t jump_distance;
        if (opcode == OP_LOOP) {
            if (new_jump_offset + 3 < new_target) {
                continue;
            }
            jump_distance = (new_jump_offset + 3) - new_target;
        } else {
            if (new_target < new_jump_offset + 3) {
                continue;
            }
            jump_distance = new_target - (new_jump_offset + 3);
        }

        if (jump_distance > UINT16_MAX) {
            continue;
        }
        write_u16_be(&chunk->code[new_jump_offset + 1], (uint16_t)jump_distance);
    }

    free(dead_bytes);
    free(prefix_removed);
    free(dead_offsets);
    free(jump_offsets);
    free(jump_targets);
    free(jump_opcodes);
    clear_reachability_state(chunk);

    return result;
}
