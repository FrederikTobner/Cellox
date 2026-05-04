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
 * @file compilation_context.h
 * @brief Accessors for shared compilation context state.
 */

#ifndef CELLOX_COMPILATION_CONTEXT_H_
#define CELLOX_COMPILATION_CONTEXT_H_

#include <frontend/internal/core_types.h>
#include <frontend/internal/expression_parser.h>
#include <frontend/internal/parser_state.h>

parser_t * compilation_context_get_parser(void);

compiler_t * compilation_context_get_current_compiler(void);
compiler_t ** compilation_context_get_current_compiler_ref(void);
void compilation_context_set_current_compiler(compiler_t * compiler);

class_compiler_t * compilation_context_get_current_class(void);
class_compiler_t ** compilation_context_get_current_class_ref(void);
void compilation_context_set_current_class(class_compiler_t * currentClass);

loop_context_t * compilation_context_get_current_loop(void);
void compilation_context_set_current_loop(loop_context_t * currentLoop);

void compilation_context_init_expression_parser(void);

#endif
