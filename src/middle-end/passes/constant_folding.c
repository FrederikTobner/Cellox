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
 * @file constant_folding.c
 * @brief Constant folding optimization pass
 */

#include "../optimization_pass.h"

#include "language-models/object.h"
#include "language-models/value.h"

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

static bool try_fold_binary_op(uint8_t op, value_t a, value_t b, value_t * result) {
    if (!IS_NUMBER(a) || !IS_NUMBER(b)) {
        return false;
    }

    double x = AS_NUMBER(a);
    double y = AS_NUMBER(b);

    switch (op) {
    case OP_ADD:
        *result = NUMBER_VAL(x + y);
        return true;
    case OP_SUBTRACT:
        *result = NUMBER_VAL(x - y);
        return true;
    case OP_MULTIPLY:
        *result = NUMBER_VAL(x * y);
        return true;
    case OP_DIVIDE:
        if (y == 0.0) {
            return false;
        }
        *result = NUMBER_VAL(x / y);
        return true;
    default:
        return false;
    }
}

pass_result_t pass_constant_folding(chunk_t * chunk) {
    pass_result_t result = {
        .pass_name = "constant_folding",
        .modified = false,
        .instructions_removed = 0,
        .constants_folded = 0,
        .branches_eliminated = 0,
    };

    for (uint32_t i = 0; i < chunk->byteCodeCount;) {
        if (chunk->code[i] != OP_CONSTANT || i + 1 >= chunk->byteCodeCount) {
            i += opcode_size(chunk, i);
            continue;
        }

        uint8_t const_idx_a = chunk->code[i + 1];
        if (const_idx_a >= chunk->constants.count) {
            i += 2;
            continue;
        }

        value_t folded_value;

        // Binary pattern: CONST a, CONST b, OP
        if (i + 4 >= chunk->byteCodeCount || chunk->code[i + 2] != OP_CONSTANT) {
            i += 2;
            continue;
        }

        uint8_t const_idx_b = chunk->code[i + 3];
        uint8_t op = chunk->code[i + 4];
        if (const_idx_b >= chunk->constants.count) {
            i += 2;
            continue;
        }

        if (!try_fold_binary_op(op, chunk->constants.values[const_idx_a], chunk->constants.values[const_idx_b],
                                &folded_value)) {
            i += 2;
            continue;
        }

        chunk->constants.values[const_idx_a] = folded_value;
        chunk_remove_bytecode(chunk, i + 2, 3); // remove CONST b + OP

        result.modified = true;
        result.constants_folded++;
        result.instructions_removed += 3;

        if (i >= 2 && chunk->code[i - 2] == OP_CONSTANT) {
            i -= 2;
            continue;
        }
    }

    return result;
}
