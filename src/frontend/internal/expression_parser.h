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
 * @file expression_parser.h
 * @brief Expression compiler API.
 */

#ifndef CELLOX_EXPRESSION_PARSER_H_
#define CELLOX_EXPRESSION_PARSER_H_

#include <frontend/internal/core_types.h>
#include <frontend/internal/parser_state.h>

typedef struct {
    parser_t * parser;
    compiler_t ** currentCompiler;
    class_compiler_t ** currentClass;
} expression_parser_context_t;

void expression_parser_init(expression_parser_context_t * context);
void expression_parser_compile_dynamic_array(bool canAssign);
void expression_parser_parse_expression(void);
uint8_t expression_parser_identifier_constant(token_t * name);
bool expression_parser_identifiers_equal(token_t * a, token_t * b);
void expression_parser_compile_named_variable(token_t name, bool canAssign);
void expression_parser_compile_variable(bool canAssign);

#endif