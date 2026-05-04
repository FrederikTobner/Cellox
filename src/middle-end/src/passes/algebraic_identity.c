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
 * @file algebraic_identity.c
 * @brief Algebraic identity simplification optimization pass
 */

#include "middle-end/optimization_pass.h"

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

static bool opcode_guarantees_numeric(chunk_t * chunk, uint32_t offset) {
    if (offset >= chunk->byteCodeCount) {
        return false;
    }

    switch (chunk->code[offset]) {
    case OP_NEGATE:
    case OP_SUBTRACT:
    case OP_MULTIPLY:
    case OP_DIVIDE:
    case OP_MODULO:
    case OP_EXPONENT:
        return true;
    case OP_CONSTANT:
        if (offset + 1 >= chunk->byteCodeCount) {
            return false;
        }
        {
            uint8_t idx = chunk->code[offset + 1];
            return idx < chunk->constants.count && IS_NUMBER(chunk->constants.values[idx]);
        }
    default:
        return false;
    }
}

pass_result_t pass_algebraic_identity(chunk_t * chunk) {
    pass_result_t result = {
        .pass_name = "algebraic_identity",
        .modified = false,
        .instructions_removed = 0,
        .constants_folded = 0,
        .branches_eliminated = 0,
    };

    uint32_t prev_start = UINT32_MAX;
    for (uint32_t i = 0; i < chunk->byteCodeCount;) {
        uint32_t size = opcode_size(chunk, i);

        // Pattern (safe): <numeric-producing expr>, CONST(0|1), OP(+,-,*,/)
        // Remove CONST+OP when operation is identity for numeric lhs.
        if (chunk->code[i] == OP_CONSTANT && i + 2 < chunk->byteCodeCount && prev_start != UINT32_MAX &&
            opcode_guarantees_numeric(chunk, prev_start)) {
            uint8_t idx = chunk->code[i + 1];
            uint8_t op = chunk->code[i + 2];

            if (idx < chunk->constants.count && IS_NUMBER(chunk->constants.values[idx])) {
                double rhs = AS_NUMBER(chunk->constants.values[idx]);
                bool identity = (op == OP_ADD && rhs == 0.0) || (op == OP_SUBTRACT && rhs == 0.0) ||
                                (op == OP_MULTIPLY && rhs == 1.0) || (op == OP_DIVIDE && rhs == 1.0);
                if (identity) {
                    chunk_remove_bytecode(chunk, i, 3); // remove CONST rhs + OP
                    result.modified = true;
                    result.instructions_removed += 3;
                    continue;
                }
            }
        }

        prev_start = i;
        i += size;
    }

    return result;
}
