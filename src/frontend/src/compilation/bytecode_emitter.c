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
 * See the <"https://www.gnu.org/licenses/gpl-3.0.html">GNU General Public  *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file bytecode_emitter.c
 * @brief Bytecode emission helpers shared by parser modules.
 */

#include <frontend/internal/compiler_ops.h>
#include <frontend/internal/compilation_context.h>

#include <stdint.h>

#include "byte-code/chunk.h"

chunk_t * compiler_current_chunk(void) {
    return &compilation_context_get_current_compiler()->function->chunk;
}

void compiler_emit_byte(uint8_t byte) {
    chunk_write(compiler_current_chunk(), byte, compilation_context_get_parser()->previous.line);
}

void compiler_emit_bytes(uint8_t byte1, uint8_t byte2) {
    compiler_emit_byte(byte1);
    compiler_emit_byte(byte2);
}

void compiler_emit_constant(value_t value) {
    compiler_emit_bytes(OP_CONSTANT, compiler_make_constant(value));
}

int32_t compiler_emit_jump(uint8_t instruction) {
    compiler_emit_byte(instruction);
    compiler_emit_byte(0xff);
    compiler_emit_byte(0xff);
    return compiler_current_chunk()->byteCodeCount - 2;
}

void compiler_emit_loop(int32_t loopStart) {
    compiler_emit_byte(OP_LOOP);
    int32_t offset = compiler_current_chunk()->byteCodeCount - loopStart + 2;
    if (offset > (int32_t)UINT16_MAX) {
        compiler_error("Loop body too large.");
    }
    compiler_emit_byte((offset >> 8) & 0xff);
    compiler_emit_byte(offset & 0xff);
}

void compiler_patch_jump(int32_t offset) {
    int32_t jump = compiler_current_chunk()->byteCodeCount - offset - 2;
    if (jump > (int32_t)UINT16_MAX) {
        compiler_error("Too much code to jump over.");
    }
    compiler_current_chunk()->code[offset] = (jump >> 8) & 0xff;
    compiler_current_chunk()->code[offset + 1] = jump & 0xff;
}

uint8_t compiler_make_constant(value_t value) {
    int32_t constant = chunk_add_constant(compiler_current_chunk(), value);
    if (constant > (int32_t)UINT8_MAX) {
        compiler_error("Too many constants in one chunk.");
        return 0;
    }
    return (uint8_t)constant;
}
