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
 * @file compiler_ops.h
 * @brief Shared compiler operation declarations used by parser modules.
 */

#ifndef CELLOX_COMPILER_OPS_H_
#define CELLOX_COMPILER_OPS_H_

#include <frontend/internal/core_types.h>

void compiler_add_local(token_t name);
uint32_t compiler_add_upvalue(compiler_t * compiler, uint8_t index, bool isLocal);
void compiler_advance(void);
void compiler_begin_scope(void);
bool compiler_check(tokentype type);
void compiler_consume(tokentype type, char const * message);
chunk_t * compiler_current_chunk(void);
void compiler_declare_variable(void);
object_function_t * compiler_end(void);
void compiler_end_scope(void);
void compiler_emit_byte(uint8_t byte);
void compiler_emit_bytes(uint8_t byte1, uint8_t byte2);
void compiler_emit_constant(value_t value);
int32_t compiler_emit_jump(uint8_t instruction);
void compiler_emit_loop(int32_t loopStart);
void compiler_emit_return(void);
CLX_PRINTF_FORMAT(1, 2) void compiler_error(char const * format, ...);
CLX_PRINTF_FORMAT(1, 2) void compiler_error_at_current(char const * format, ...);
void compiler_init(compiler_t * compiler, function_type type);
void compiler_mark_initialized(void);
uint8_t compiler_make_constant(value_t value);
bool compiler_match_token(tokentype type);
void compiler_patch_jump(int32_t offset);
token_t compiler_synthetic_token(char const * text);

#endif