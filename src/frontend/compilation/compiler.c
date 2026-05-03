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
 * @file compiler.c
 * @brief Compiler orchestration and shared compiler operations.
 */

#include "compiler.h"

#include "compiler_ops.h"
#include "compilation_context.h"
#include "frontend/parsing/statement_parser.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#ifdef DEBUG_PRINT_CODE
#include "byte-code/chunk_disassembler.h"
#endif
#include "backend/garbage_collector.h"
#include "backend/memory_mutator.h"
#include "middle-end/chunk_optimizer.h"
#include "frontend/lexical_analysis/lexer.h"

static void compiler_error_at(token_t * token, char const * format, va_list args);
void compiler_error_at_current(char const * format, ...);

object_function_t * compiler_compile(char const * program) {
    parser_t * parser = compilation_context_get_parser();

    lexer_init(program);
    compilation_context_init_expression_parser();

    compiler_t compiler;
    compiler_init(&compiler, TYPE_SCRIPT);
    parser->hadError = false;
    parser->panicMode = false;

    compiler_advance();
    statement_parser_parse_program();

    object_function_t * function = compiler_end();
    return parser->hadError ? NULL : function;
}

void compiler_mark_roots(void) {
    compiler_t * compiler = compilation_context_get_current_compiler();
    while (compiler) {
        garbage_collector_mark_object((object_t *)compiler->function);
        compiler = compiler->enclosing;
    }
}

uint32_t compiler_add_upvalue(compiler_t * compiler, uint8_t index, bool isLocal) {
    uint32_t upvalueCount = compiler->function->upvalueCount;

    for (uint32_t i = 0; i < upvalueCount; i++) {
        upvalue_t * upvalue = &compiler->upvalues[i];
        if (upvalue->index == index && upvalue->isLocal == isLocal) {
            return i;
        }
    }
    if (upvalueCount == UINT8_COUNT) {
        compiler_error("Too many closure variables in function.");
        return 0;
    }

    compiler->upvalues[upvalueCount].isLocal = isLocal;
    compiler->upvalues[upvalueCount].index = index;
    return compiler->function->upvalueCount++;
}

void compiler_advance(void) {
    parser_t * parser = compilation_context_get_parser();

    parser->previous = parser->current;
    for (;;) {
        parser->current = lexer_scan_token();
        if (parser->current.type != TOKEN_ERROR) {
            break;
        }
        compiler_error_at_current(parser->current.start);
    }
}

bool compiler_check(tokentype type) {
    return compilation_context_get_parser()->current.type == type;
}

void compiler_consume(tokentype type, char const * message) {
    if (compilation_context_get_parser()->current.type == type) {
        compiler_advance();
        return;
    }
    compiler_error_at_current(message);
}

object_function_t * compiler_end(void) {
    parser_t * parser = compilation_context_get_parser();
    compiler_t * currentCompiler = compilation_context_get_current_compiler();

    compiler_emit_return();
    object_function_t * function = currentCompiler->function;

#ifdef DEBUG_PRINT_CODE
    if (!parser->hadError) {
        chunk_disassembler_disassemble_chunk(compiler_current_chunk(),
                                             function->name != NULL ? function->name->chars : "main",
                                             function->arity);
    }
#endif

    compilation_context_set_current_compiler(currentCompiler->enclosing);
    chunk_optimizer_optimize_chunk(&function->chunk);
    return function;
}

void compiler_emit_return(void) {
    if (compilation_context_get_current_compiler()->type == TYPE_INITIALIZER) {
        compiler_emit_bytes(OP_GET_LOCAL, 0);
    } else {
        compiler_emit_byte(OP_NULL);
    }
    compiler_emit_byte(OP_RETURN);
}

void compiler_error(char const * format, ...) {
    va_list args;
    va_start(args, format);
    compiler_error_at(&compilation_context_get_parser()->previous, format, args);
    va_end(args);
}

static void compiler_error_at(token_t * token, char const * format, va_list args) {
    parser_t * parser = compilation_context_get_parser();

    if (parser->panicMode) {
        return;
    }
    parser->panicMode = true;

    fprintf(stderr, "[line %d] Error", token->line);
    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end: ");
    } else {
        fprintf(stderr, " at '%.*s': ", token->length, token->start);
    }
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
    parser->hadError = true;
}

void compiler_error_at_current(char const * format, ...) {
    va_list args;
    va_start(args, format);
    compiler_error_at(&compilation_context_get_parser()->current, format, args);
    va_end(args);
}

void compiler_init(compiler_t * compiler, function_type type) {
    parser_t * parser = compilation_context_get_parser();
    compiler_t * currentCompiler = compilation_context_get_current_compiler();

    compiler->enclosing = currentCompiler;
    compiler->function = NULL;
    compiler->type = type;
    compiler->localCount = compiler->scopeDepth = 0;
    compiler->function = object_new_function();

    compilation_context_set_current_compiler(compiler);
    if (type != TYPE_SCRIPT) {
        compilation_context_get_current_compiler()->function->name =
            object_copy_string(parser->previous.start, parser->previous.length, false);
    }

    local_t * local = &compilation_context_get_current_compiler()->locals
        [compilation_context_get_current_compiler()->localCount++];
    local->depth = 0;
    local->isCaptured = false;
    if (type != TYPE_FUNCTION) {
        local->name.start = "this";
        local->name.length = 4;
    } else {
        local->name.start = "";
        local->name.length = 0;
    }
}

bool compiler_match_token(tokentype type) {
    if (!compiler_check(type)) {
        return false;
    }
    compiler_advance();
    return true;
}

token_t compiler_synthetic_token(char const * text) {
    token_t token;
    token.type = TOKEN_IDENTIFIER;
    token.start = text;
    token.length = (uint32_t)strlen(text);
    token.line = compilation_context_get_parser()->previous.line;
    return token;
}
