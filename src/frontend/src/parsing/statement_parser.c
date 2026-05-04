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
 * @file statement_parser.c
 * @brief Statement and declaration parser implementation.
 */

#include <frontend/internal/statement_parser.h>

#include <frontend/internal/compiler_ops.h>
#include <frontend/internal/compilation_context.h>
#include <frontend/internal/expression_parser.h>

#include <string.h>

#include "backend/virtual_machine.h"

#define PARSER (*compilation_context_get_parser())
#define CURRENT (compilation_context_get_current_compiler())
#define CURRENT_CLASS (compilation_context_get_current_class())
#define CURRENT_LOOP (compilation_context_get_current_loop())

static void compiler_block(void);
static void compiler_break_statement(void);
static void compiler_class_declaration(void);
static void compiler_continue_statement(void);
static void compiler_declaration(void);
static void compiler_define_variable(uint8_t global);
static void compiler_do_while_statement(void);
static void compiler_error_declaration(void);
static void compiler_emit_loop_cleanup_pops(void);
static void compiler_expression_statement(void);
static void compiler_for_statement(void);
static void compiler_function(function_type type);
static void compiler_function_declaration(void);
static void compiler_if_statement(void);
static void compiler_iferror_statement(void);
static void compiler_method(void);
static uint8_t compiler_parse_variable(char const * errorMessage);
static void compiler_return_statement(void);
static void compiler_statement(void);
static void compiler_synchronize(void);
static void compiler_throw_statement(void);
static void compiler_var_declaration(void);
static void compiler_while_statement(void);

void statement_parser_parse_program(void) {
    while (!compiler_match_token(TOKEN_EOF)) {
        compiler_declaration();
    }
}

void compiler_add_local(token_t name) {
    if (CURRENT->localCount == UINT8_COUNT) {
        compiler_error("Too many local variables in function.");
        return;
    }
    local_t * local = &CURRENT->locals[CURRENT->localCount++];
    local->name = name;
    local->depth = -1;
    local->isCaptured = false;
}

void compiler_begin_scope(void) {
    CURRENT->scopeDepth++;
}

static void compiler_block(void) {
    while (!compiler_check(TOKEN_RIGHT_BRACE) && !compiler_check(TOKEN_EOF)) {
        compiler_declaration();
    }
    compiler_consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void compiler_class_declaration(void) {
    compiler_consume(TOKEN_IDENTIFIER, "Expect class name.");
    token_t className = PARSER.previous;
    uint8_t nameConstant = expression_parser_identifier_constant(&PARSER.previous);
    compiler_declare_variable();
    compiler_emit_bytes(OP_CLASS, nameConstant);
    compiler_define_variable(nameConstant);

    class_compiler_t classCompiler;
    classCompiler.hasSuperclass = false;
    classCompiler.enclosing = CURRENT_CLASS;
    compilation_context_set_current_class(&classCompiler);

    if (compiler_match_token(TOKEN_DOUBLEDOT)) {
        compiler_consume(TOKEN_IDENTIFIER, "Expect superclass name.");
        expression_parser_compile_variable(false);
        if (expression_parser_identifiers_equal(&className, &PARSER.previous)) {
            compiler_error("A class can't inherit from itself.");
        }
        compiler_begin_scope();
        compiler_add_local(compiler_synthetic_token("super"));
        compiler_define_variable(0);
        expression_parser_compile_named_variable(className, false);
        compiler_emit_byte(OP_INHERIT);
        classCompiler.hasSuperclass = true;
    }

    expression_parser_compile_named_variable(className, false);
    compiler_consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");
    while (!compiler_check(TOKEN_RIGHT_BRACE) && !compiler_check(TOKEN_EOF)) {
        compiler_method();
    }
    compiler_consume(TOKEN_RIGHT_BRACE, "Expect '}' after class body.");
    compiler_emit_byte(OP_POP);
    if (classCompiler.hasSuperclass) {
        compiler_end_scope();
    }
    compilation_context_set_current_class(classCompiler.enclosing);
}

static void compiler_declaration(void) {
    if (compiler_match_token(TOKEN_CLASS)) {
        compiler_class_declaration();
    } else if (compiler_match_token(TOKEN_ERROR_DECL)) {
        compiler_error_declaration();
    } else if (compiler_match_token(TOKEN_FUN)) {
        compiler_function_declaration();
    } else if (compiler_match_token(TOKEN_VAR)) {
        compiler_var_declaration();
    } else {
        compiler_statement();
    }

    if (PARSER.panicMode) {
        compiler_synchronize();
    }
}

void compiler_declare_variable(void) {
    if (!CURRENT->scopeDepth) {
        return;
    }

    token_t * name = &PARSER.previous;
    for (int32_t i = CURRENT->localCount - 1; i >= 0; i--) {
        local_t * local = &CURRENT->locals[i];
        if (local->depth != -1 && local->depth < CURRENT->scopeDepth) {
            break;
        }

        if (expression_parser_identifiers_equal(name, &local->name)) {
            compiler_error("Already a variable with this name in this scope.");
        }
    }
    compiler_add_local(*name);
}

static void compiler_define_variable(uint8_t global) {
    if (CURRENT->scopeDepth > 0) {
        compiler_mark_initialized();
        return;
    }
    compiler_emit_bytes(OP_DEFINE_GLOBAL, global);
}

static void compiler_emit_loop_cleanup_pops(void) {
    for (int32_t i = CURRENT->localCount - 1; i >= CURRENT_LOOP->localCountAtLoop; i--) {
        if (CURRENT->locals[i].isCaptured) {
            compiler_emit_byte(OP_CLOSE_UPVALUE);
        } else {
            compiler_emit_byte(OP_POP);
        }
    }
}

static void compiler_break_statement(void) {
    if (!CURRENT_LOOP) {
        compiler_error("Can't use 'break' outside of a loop.");
        return;
    }
    compiler_consume(TOKEN_SEMICOLON, "Expect ';' after 'break'.");
    if (CURRENT_LOOP->breakJumpCount == LOOP_MAX_PENDING_JUMPS) {
        compiler_error("Too many 'break' statements in a single loop.");
        return;
    }
    compiler_emit_loop_cleanup_pops();
    CURRENT_LOOP->breakJumps[CURRENT_LOOP->breakJumpCount++] = compiler_emit_jump(OP_JUMP);
}

static void compiler_continue_statement(void) {
    if (!CURRENT_LOOP) {
        compiler_error("Can't use 'continue' outside of a loop.");
        return;
    }
    compiler_consume(TOKEN_SEMICOLON, "Expect ';' after 'continue'.");
    compiler_emit_loop_cleanup_pops();
    if (CURRENT_LOOP->continueTarget >= 0) {
        compiler_emit_loop(CURRENT_LOOP->continueTarget);
    } else {
        if (CURRENT_LOOP->continueJumpCount == LOOP_MAX_PENDING_JUMPS) {
            compiler_error("Too many 'continue' statements in a single loop.");
            return;
        }
        CURRENT_LOOP->continueJumps[CURRENT_LOOP->continueJumpCount++] = compiler_emit_jump(OP_JUMP);
    }
}

static void compiler_do_while_statement(void) {
    loop_context_t loopContext;
    loopContext.enclosing = CURRENT_LOOP;
    loopContext.continueTarget = -1;
    loopContext.continueJumpCount = 0;
    loopContext.breakJumpCount = 0;
    loopContext.localCountAtLoop = CURRENT->localCount;
    compilation_context_set_current_loop(&loopContext);

    int32_t loopStart = compiler_current_chunk()->byteCodeCount;
    compiler_statement();

    for (int32_t i = 0; i < loopContext.continueJumpCount; i++) {
        compiler_patch_jump(loopContext.continueJumps[i]);
    }

    compiler_consume(TOKEN_WHILE, "Expect 'while' after 'do'.");
    compiler_consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression_parser_parse_expression();
    compiler_consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
    int32_t exitJump = compiler_emit_jump(OP_JUMP_IF_FALSE);
    compiler_emit_byte(OP_POP);
    compiler_emit_loop(loopStart);
    compiler_patch_jump(exitJump);
    compiler_emit_byte(OP_POP);
    compiler_consume(TOKEN_SEMICOLON, "Expect ';' after 'while'.");

    for (int32_t i = 0; i < loopContext.breakJumpCount; i++) {
        compiler_patch_jump(loopContext.breakJumps[i]);
    }
    compilation_context_set_current_loop(loopContext.enclosing);
}

void compiler_end_scope(void) {
    CURRENT->scopeDepth--;
    while (CURRENT->localCount > 0 && CURRENT->locals[CURRENT->localCount - 1].depth > CURRENT->scopeDepth) {
        if (CURRENT->locals[CURRENT->localCount - 1].isCaptured) {
            compiler_emit_byte(OP_CLOSE_UPVALUE);
        } else {
            compiler_emit_byte(OP_POP);
        }
        CURRENT->localCount--;
    }
}

static void compiler_error_declaration(void) {
    uint8_t global = compiler_parse_variable("Expect error set name.");
    token_t errorSetNameToken = PARSER.previous;
    object_string_t * errorSetName =
        object_copy_string(errorSetNameToken.start, errorSetNameToken.length, false);
    object_error_set_t * errorSet = object_new_error_set(errorSetName);

    compiler_emit_constant(OBJECT_VAL(errorSet));
    compiler_define_variable(global);

    compiler_consume(TOKEN_LEFT_BRACE, "Expect '{' before error set body.");
    if (!compiler_check(TOKEN_RIGHT_BRACE)) {
        do {
            if (compiler_check(TOKEN_RIGHT_BRACE)) {
                break;
            }
            compiler_consume(TOKEN_IDENTIFIER, "Expect error variant name.");
            token_t variantNameToken = PARSER.previous;
            object_string_t * variantName =
                object_copy_string(variantNameToken.start, variantNameToken.length, false);
            object_error_value_t * errorValue = object_new_error_value(errorSet, variantName);
            value_hash_table_set(&errorSet->variants, variantName, OBJECT_VAL(errorValue));
        } while (compiler_match_token(TOKEN_COMMA));
    }
    compiler_consume(TOKEN_RIGHT_BRACE, "Expect '}' after error set body.");
}

static void compiler_expression_statement(void) {
    expression_parser_parse_expression();
    compiler_consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    compiler_emit_byte(OP_POP);
}

static void compiler_for_statement(void) {
    compiler_begin_scope();
    compiler_consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

    if (compiler_match_token(TOKEN_VAR)) {
        compiler_var_declaration();
    } else if (!compiler_match_token(TOKEN_SEMICOLON)) {
        compiler_expression_statement();
    }

    int32_t loopStart = compiler_current_chunk()->byteCodeCount;
    int32_t exitJump = -1;

    if (!compiler_match_token(TOKEN_SEMICOLON)) {
        expression_parser_parse_expression();
        compiler_consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");
        exitJump = compiler_emit_jump(OP_JUMP_IF_FALSE);
        compiler_emit_byte(OP_POP);
    }

    loop_context_t loopContext;
    loopContext.enclosing = CURRENT_LOOP;
    loopContext.continueTarget = loopStart;
    loopContext.continueJumpCount = 0;
    loopContext.breakJumpCount = 0;
    loopContext.localCountAtLoop = CURRENT->localCount;
    compilation_context_set_current_loop(&loopContext);

    if (!compiler_match_token(TOKEN_RIGHT_PAREN)) {
        int32_t bodyJump = compiler_emit_jump(OP_JUMP);
        int32_t incrementStart = compiler_current_chunk()->byteCodeCount;
        expression_parser_parse_expression();
        compiler_emit_byte(OP_POP);
        compiler_consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");
        compiler_emit_loop(loopStart);
        loopStart = incrementStart;
        loopContext.continueTarget = incrementStart;
        compiler_patch_jump(bodyJump);
    }

    compiler_statement();
    compiler_emit_loop(loopStart);

    if (exitJump != -1) {
        compiler_patch_jump(exitJump);
        compiler_emit_byte(OP_POP);
    }

    for (int32_t i = 0; i < loopContext.breakJumpCount; i++) {
        compiler_patch_jump(loopContext.breakJumps[i]);
    }
    compilation_context_set_current_loop(loopContext.enclosing);

    compiler_end_scope();
}

static void compiler_function(function_type type) {
    compiler_t compiler;
    compiler_init(&compiler, type);
    compiler_begin_scope();

    compiler_consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");
    if (!compiler_check(TOKEN_RIGHT_PAREN)) {
        do {
            CURRENT->function->arity++;
            if (CURRENT->function->arity > 255) {
                compiler_error_at_current("Can't have more than 255 parameters.");
            }
            uint8_t constant = compiler_parse_variable("Expect parameter name.");
            compiler_define_variable(constant);
        } while (compiler_match_token(TOKEN_COMMA));
    }
    compiler_consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    compiler_consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    compiler_block();

    object_function_t * function = compiler_end();
    compiler_emit_bytes(OP_CLOSURE, compiler_make_constant(OBJECT_VAL(function)));
    for (int32_t i = 0; i < function->upvalueCount; i++) {
        compiler_emit_byte(compiler.upvalues[i].isLocal ? 1 : 0);
        compiler_emit_byte(compiler.upvalues[i].index);
    }
}

static void compiler_function_declaration(void) {
    uint8_t global = compiler_parse_variable("Expect function name.");
    compiler_mark_initialized();
    compiler_function(TYPE_FUNCTION);
    compiler_define_variable(global);
}

static void compiler_if_statement(void) {
    compiler_consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression_parser_parse_expression();
    compiler_consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
    int32_t thenJump = compiler_emit_jump(OP_JUMP_IF_FALSE);
    compiler_emit_byte(OP_POP);
    compiler_statement();
    int32_t elseJump = compiler_emit_jump(OP_JUMP);
    compiler_patch_jump(thenJump);
    compiler_emit_byte(OP_POP);
    if (compiler_match_token(TOKEN_ELSE)) {
        compiler_statement();
    }
    compiler_patch_jump(elseJump);
}

void compiler_mark_initialized(void) {
    if (!CURRENT->scopeDepth) {
        return;
    }
    CURRENT->locals[CURRENT->localCount - 1].depth = CURRENT->scopeDepth;
}

static void compiler_method(void) {
    compiler_consume(TOKEN_IDENTIFIER, "Expect method name.");
    uint8_t constant = expression_parser_identifier_constant(&PARSER.previous);
    function_type type = TYPE_METHOD;
    if (PARSER.previous.length == 4 && !memcmp(PARSER.previous.start, "init", 4)) {
        type = TYPE_INITIALIZER;
    }
    compiler_function(type);
    compiler_emit_bytes(OP_METHOD, constant);
}

static uint8_t compiler_parse_variable(char const * errorMessage) {
    compiler_consume(TOKEN_IDENTIFIER, errorMessage);
    compiler_declare_variable();
    if (CURRENT->scopeDepth > 0) {
        return 0;
    }
    return expression_parser_identifier_constant(&PARSER.previous);
}

static void compiler_return_statement(void) {
    if (CURRENT->type == TYPE_SCRIPT) {
        compiler_error("You can't use return from top-level code.");
    }
    if (compiler_match_token(TOKEN_SEMICOLON)) {
        compiler_emit_return();
    } else {
        if (CURRENT->type == TYPE_INITIALIZER) {
            compiler_error("Can't return a value from an initializer. An initializer in cellox is not permitted to "
                           "return a value.");
        }
        expression_parser_parse_expression();
        compiler_consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
        compiler_emit_byte(OP_RETURN);
    }
}

static void compiler_statement(void) {
    if (compiler_match_token(TOKEN_FOR)) {
        compiler_for_statement();
    } else if (compiler_match_token(TOKEN_IF)) {
        compiler_if_statement();
    } else if (compiler_match_token(TOKEN_RETURN)) {
        compiler_return_statement();
    } else if (compiler_match_token(TOKEN_BREAK)) {
        compiler_break_statement();
    } else if (compiler_match_token(TOKEN_CONTINUE)) {
        compiler_continue_statement();
    } else if (compiler_match_token(TOKEN_WHILE)) {
        compiler_while_statement();
    } else if (compiler_match_token(TOKEN_DO)) {
        compiler_do_while_statement();
    } else if (compiler_match_token(TOKEN_IFERROR)) {
        compiler_iferror_statement();
    } else if (compiler_match_token(TOKEN_THROW)) {
        compiler_throw_statement();
    } else if (compiler_match_token(TOKEN_LEFT_BRACE)) {
        compiler_begin_scope();
        compiler_block();
        compiler_end_scope();
    } else {
        compiler_expression_statement();
    }
}

static void compiler_throw_statement(void) {
    if (CURRENT->type == TYPE_SCRIPT) {
        compiler_error("You can't use throw from top-level code.");
    }
    expression_parser_parse_expression();
    compiler_consume(TOKEN_SEMICOLON, "Expect ';' after throw value.");
    compiler_emit_byte(OP_RESULT_WRAP_ERR);
    compiler_emit_byte(OP_RETURN);
}

static void compiler_iferror_statement(void) {
    // iferror <expr> |err| { ... } else |val| { ... }
    expression_parser_parse_expression(); // push result value
    compiler_emit_byte(OP_RESULT_IS_ERROR); // peek result, push bool
    int32_t elseBranch = compiler_emit_jump(OP_JUMP_IF_FALSE);

    // error branch
    compiler_emit_byte(OP_POP); // pop true
    compiler_emit_byte(OP_RESULT_UNWRAP_ERROR); // pop result, push error payload
    compiler_consume(TOKEN_PIPE, "Expect '|' before error variable name.");
    compiler_consume(TOKEN_IDENTIFIER, "Expect error variable name.");
    token_t errName = PARSER.previous;
    compiler_consume(TOKEN_PIPE, "Expect '|' after error variable name.");
    compiler_consume(TOKEN_LEFT_BRACE, "Expect '{' before iferror error body.");
    compiler_begin_scope();
    compiler_add_local(errName);
    compiler_mark_initialized();
    compiler_block();
    compiler_end_scope();
    int32_t endJump = compiler_emit_jump(OP_JUMP);

    // success (else) branch
    compiler_patch_jump(elseBranch);
    compiler_emit_byte(OP_POP); // pop false
    compiler_emit_byte(OP_RESULT_UNWRAP); // pop result, push success payload
    compiler_consume(TOKEN_ELSE, "Expect 'else' after iferror error body.");
    compiler_consume(TOKEN_PIPE, "Expect '|' before success variable name.");
    compiler_consume(TOKEN_IDENTIFIER, "Expect success variable name.");
    token_t valName = PARSER.previous;
    compiler_consume(TOKEN_PIPE, "Expect '|' after success variable name.");
    compiler_consume(TOKEN_LEFT_BRACE, "Expect '{' before iferror success body.");
    compiler_begin_scope();
    compiler_add_local(valName);
    compiler_mark_initialized();
    compiler_block();
    compiler_end_scope();

    compiler_patch_jump(endJump);
}

static void compiler_synchronize(void) {
    PARSER.panicMode = false;
    while (PARSER.current.type != TOKEN_EOF) {
        if (PARSER.previous.type == TOKEN_SEMICOLON) {
            return;
        }
        compiler_advance();
        switch (PARSER.current.type) {
        case TOKEN_CLASS:
        case TOKEN_FUN:
        case TOKEN_VAR:
        case TOKEN_ERROR_DECL:
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_IFERROR:
        case TOKEN_THROW:
        case TOKEN_WHILE:
        case TOKEN_RETURN:
        case TOKEN_BREAK:
        case TOKEN_CONTINUE:
            return;

        default:
            compiler_advance();
        }
    }
}

static void compiler_var_declaration(void) {
    uint8_t global = compiler_parse_variable("Expect variable name.");
    if (compiler_match_token(TOKEN_EQUAL)) {
        if (!compiler_match_token(TOKEN_LEFT_BRACE)) {
            expression_parser_parse_expression();
        } else {
            expression_parser_compile_dynamic_array(false);
        }
    } else {
        compiler_emit_byte(OP_NULL);
    }
    compiler_consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    compiler_define_variable(global);
}

static void compiler_while_statement(void) {
    int32_t loopStart = compiler_current_chunk()->byteCodeCount;

    loop_context_t loopContext;
    loopContext.enclosing = CURRENT_LOOP;
    loopContext.continueTarget = loopStart;
    loopContext.continueJumpCount = 0;
    loopContext.breakJumpCount = 0;
    loopContext.localCountAtLoop = CURRENT->localCount;
    compilation_context_set_current_loop(&loopContext);

    compiler_consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression_parser_parse_expression();
    compiler_consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
    int32_t exitJump = compiler_emit_jump(OP_JUMP_IF_FALSE);
    compiler_emit_byte(OP_POP);
    compiler_statement();
    compiler_emit_loop(loopStart);
    compiler_patch_jump(exitJump);
    compiler_emit_byte(OP_POP);

    for (int32_t i = 0; i < loopContext.breakJumpCount; i++) {
        compiler_patch_jump(loopContext.breakJumps[i]);
    }
    compilation_context_set_current_loop(loopContext.enclosing);
}
