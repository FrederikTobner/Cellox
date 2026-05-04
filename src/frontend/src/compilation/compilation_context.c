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
 * @file compilation_context.c
 * @brief Shared compilation context state implementation.
 */

#include <frontend/internal/compilation_context.h>

static parser_t parser;
static compiler_t * currentCompiler = NULL;
static class_compiler_t * currentClass = NULL;
static loop_context_t * currentLoop = NULL;
static expression_parser_context_t expressionParserContext;

parser_t * compilation_context_get_parser(void) {
    return &parser;
}

compiler_t * compilation_context_get_current_compiler(void) {
    return currentCompiler;
}

compiler_t ** compilation_context_get_current_compiler_ref(void) {
    return &currentCompiler;
}

void compilation_context_set_current_compiler(compiler_t * compiler) {
    currentCompiler = compiler;
}

class_compiler_t * compilation_context_get_current_class(void) {
    return currentClass;
}

class_compiler_t ** compilation_context_get_current_class_ref(void) {
    return &currentClass;
}

void compilation_context_set_current_class(class_compiler_t * classCompiler) {
    currentClass = classCompiler;
}

loop_context_t * compilation_context_get_current_loop(void) {
    return currentLoop;
}

void compilation_context_set_current_loop(loop_context_t * loopContext) {
    currentLoop = loopContext;
}

void compilation_context_init_expression_parser(void) {
    expressionParserContext.parser = &parser;
    expressionParserContext.currentCompiler = &currentCompiler;
    expressionParserContext.currentClass = &currentClass;
    expression_parser_init(&expressionParserContext);
}
