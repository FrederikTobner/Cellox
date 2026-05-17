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
 * @file expression_parser.c
 * @brief File containing expression-related compiler implementation.
 */

#include <frontend/internal/compiler_ops.h>
#include <frontend/internal/expression_parser.h>

#include <stdlib.h>
#include <string.h>

#include "backend/virtual_machine.h"
#include <frontend/internal/lexer.h>
#include <frontend/internal/parse_rules.h>

static expression_parser_context_t * expressionContext = NULL;

#define PARSER        (*(expressionContext->parser))
#define CURRENT       (*(expressionContext->currentCompiler))
#define CURRENT_CLASS (*(expressionContext->currentClass))

static inline void expression_parser_advance_token(void) {
    compiler_advance();
}

static inline bool expression_parser_check_token(tokentype type) {
    return compiler_check(type);
}

static inline void expression_parser_consume_token(tokentype type, char const * message) {
    compiler_consume(type, message);
}

static inline void expression_parser_emit_byte(uint8_t byte) {
    compiler_emit_byte(byte);
}

static inline void expression_parser_emit_bytes(uint8_t byte1, uint8_t byte2) {
    compiler_emit_bytes(byte1, byte2);
}

static inline void expression_parser_emit_constant_value(value_t value) {
    compiler_emit_constant(value);
}

static inline int32_t expression_parser_emit_jump_offset(uint8_t instruction) {
    return compiler_emit_jump(instruction);
}

static inline void expression_parser_patch_jump_offset(int32_t offset) {
    compiler_patch_jump(offset);
}

static inline bool expression_parser_match_token(tokentype type) {
    return compiler_match_token(type);
}

static inline uint32_t expression_parser_add_upvalue(compiler_t * compiler, uint8_t index, bool isLocal) {
    return compiler_add_upvalue(compiler, index, isLocal);
}

static inline uint8_t expression_parser_make_constant(value_t value) {
    return compiler_make_constant(value);
}

static inline token_t expression_parser_make_synthetic_token(char const * text) {
    return compiler_synthetic_token(text);
}

static void expression_parser_error(char const * message) {
    compiler_error("%s", message);
}

static void expression_parser_and(bool canAssign);
static uint8_t expression_parser_argument_list(void);
static void expression_parser_binary(bool canAssign);
static inline void expression_parser_binary_number(bool canAssign);
static inline void expression_parser_call(bool canAssign);
static void expression_parser_dot(bool canAssign);
static uint8_t expression_parser_dynamic_array_argument_list(void);
static inline parse_rule_t * expression_parser_get_rule(tokentype type);
static void expression_parser_grouping(bool canAssign);
static inline void expression_parser_hex_number(bool canAssign);
static void expression_parser_index_of(bool canAssign, uint8_t getOp, uint32_t arg);
static void expression_parser_literal(bool canAssign);
static void expression_parser_nondirect_assignment(uint8_t assignmentType, uint8_t getOp, uint8_t setOp, uint8_t arg);
static void expression_parser_nondirect_property_assignment(uint8_t assignmentType, uint8_t name);
static inline void expression_parser_number(bool canAssign);
static void expression_parser_or(bool canAssign);
static void expression_parser_parse_precedence(precedence precedence);
static int32_t expression_parser_resolve_local(compiler_t * compiler, token_t * name);
static int32_t expression_parser_resolve_upvalue(compiler_t * compiler, token_t * name);
static void expression_parser_string(bool canAssign);
static void expression_parser_super(bool canAssign);
static void expression_parser_this(bool canAssign);
static void expression_parser_try(bool canAssign);
static void expression_parser_must(bool canAssign);
static void expression_parser_catch(bool canAssign);
static void expression_parser_iferror_expression(bool canAssign);
static void expression_parser_parse_pipe_bound_handler(precedence handlerPrecedence);
static void expression_parser_parse_handler_block(void);
static void expression_parser_unary(bool canAssign);

/// ParseRules for the tokens of cellox
static parse_rule_t rules[] = {
    [TOKEN_AND] = {.prefix = NULL, .infix = expression_parser_and, .precedence = PREC_AND},
    [TOKEN_BANG] = {.prefix = expression_parser_unary, .infix = NULL, .precedence = PREC_UNARY},
    [TOKEN_BANG_EQUAL] = {.prefix = NULL, .infix = expression_parser_binary, .precedence = PREC_EQUALITY},
    [TOKEN_BINARY_NUMBER] = {.prefix = expression_parser_binary_number, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_CLASS] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_COMMA] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_BREAK] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_CONTINUE] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_DO] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_ERROR_DECL] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_DOT] = {.prefix = NULL, .infix = expression_parser_dot, .precedence = PREC_CALL},
    [TOKEN_EOF] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_ELSE] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_EQUAL] = {.prefix = NULL, .infix = NULL, .precedence = PREC_ASSIGNMENT},
    [TOKEN_EQUAL_EQUAL] = {.prefix = NULL, .infix = expression_parser_binary, .precedence = PREC_EQUALITY},
    [TOKEN_ERROR] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_FALSE] = {.prefix = expression_parser_literal, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_FOR] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_CATCH] = {.prefix = NULL, .infix = expression_parser_catch, .precedence = PREC_OR},
    [TOKEN_FUN] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_GREATER] = {.prefix = NULL, .infix = expression_parser_binary, .precedence = PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {.prefix = NULL, .infix = expression_parser_binary, .precedence = PREC_COMPARISON},
    [TOKEN_HEX_NUMBER] = {.prefix = expression_parser_hex_number, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_IDENTIFIER] = {.prefix = expression_parser_compile_variable, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_IF] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_LEFT_BRACE] = {.prefix = expression_parser_compile_dynamic_array, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_LEFT_PAREN] = {.prefix = expression_parser_grouping,
                          .infix = expression_parser_call,
                          .precedence = PREC_CALL},
    [TOKEN_LEFT_BRACKET] = {.prefix = NULL, .infix = NULL, .precedence = PREC_CALL},
    [TOKEN_LESS] = {.prefix = NULL, .infix = expression_parser_binary, .precedence = PREC_COMPARISON},
    [TOKEN_LESS_EQUAL] = {.prefix = NULL, .infix = expression_parser_binary, .precedence = PREC_COMPARISON},
    [TOKEN_MODULO] = {.prefix = NULL, .infix = expression_parser_binary, .precedence = PREC_FACTOR},
    [TOKEN_MODULO_EQUAL] = {.prefix = NULL, .infix = NULL, .precedence = PREC_ASSIGNMENT},
    [TOKEN_MINUS] = {.prefix = expression_parser_unary, .infix = expression_parser_binary, .precedence = PREC_TERM},
    [TOKEN_MINUS_EQUAL] = {.prefix = NULL, .infix = NULL, .precedence = PREC_ASSIGNMENT},
    [TOKEN_NULL] = {.prefix = expression_parser_literal, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_NUMBER] = {.prefix = expression_parser_number, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_PLUS] = {.prefix = NULL, .infix = expression_parser_binary, .precedence = PREC_TERM},
    [TOKEN_PLUS_EQUAL] = {.prefix = NULL, .infix = NULL, .precedence = PREC_ASSIGNMENT},
    [TOKEN_OR] = {.prefix = NULL, .infix = expression_parser_or, .precedence = PREC_OR},
    [TOKEN_PRINT] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_RANGE] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_RETURN] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_TRY] = {.prefix = expression_parser_try, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_RIGHT_BRACE] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_RIGHT_PAREN] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_RIGHT_BRACKET] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_SEMICOLON] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_SLASH] = {.prefix = NULL, .infix = expression_parser_binary, .precedence = PREC_FACTOR},
    [TOKEN_SLASH_EQUAL] = {.prefix = NULL, .infix = NULL, .precedence = PREC_ASSIGNMENT},
    [TOKEN_STAR] = {.prefix = NULL, .infix = expression_parser_binary, .precedence = PREC_FACTOR},
    [TOKEN_STAR_EQUAL] = {.prefix = NULL, .infix = NULL, .precedence = PREC_ASSIGNMENT},
    [TOKEN_STAR_STAR] = {.prefix = NULL, .infix = expression_parser_binary, .precedence = PREC_FACTOR},
    [TOKEN_STRING] = {.prefix = expression_parser_string, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_SUPER] = {.prefix = expression_parser_super, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_THIS] = {.prefix = expression_parser_this, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_MUST] = {.prefix = expression_parser_must, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_TRUE] = {.prefix = expression_parser_literal, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_VAR] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_IFERROR] = {.prefix = expression_parser_iferror_expression, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_THROW] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_PIPE] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE},
    [TOKEN_WHILE] = {.prefix = NULL, .infix = NULL, .precedence = PREC_NONE}};

void expression_parser_init(expression_parser_context_t * context) {
    expressionContext = context;
}

static void expression_parser_and(bool canAssign) {
    (void)canAssign;
    int32_t endJump = expression_parser_emit_jump_offset(OP_JUMP_IF_FALSE);
    expression_parser_emit_byte(OP_POP);
    expression_parser_parse_precedence(PREC_AND);
    expression_parser_patch_jump_offset(endJump);
}

static uint8_t expression_parser_argument_list(void) {
    uint8_t argCount = 0;
    if (!expression_parser_check_token(TOKEN_RIGHT_PAREN)) {
        do {
            if (argCount == 255) {
                expression_parser_advance_token();
                expression_parser_error("Can't have more than 255 arguments in a function call.");
            }
            expression_parser_parse_expression();
            argCount++;
        } while (expression_parser_match_token(TOKEN_COMMA));
    }
    expression_parser_consume_token(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
    return argCount;
}

static void expression_parser_binary(bool canAssign) {
    (void)canAssign;
    tokentype operatorType = PARSER.previous.type;
    parse_rule_t * rule = expression_parser_get_rule(operatorType);
    expression_parser_parse_precedence((precedence)(rule->precedence + 1));

    switch (operatorType) {
    case TOKEN_BANG_EQUAL:
        expression_parser_emit_bytes(OP_EQUAL, OP_NOT);
        break;
    case TOKEN_EQUAL_EQUAL:
        expression_parser_emit_byte(OP_EQUAL);
        break;
    case TOKEN_GREATER:
        expression_parser_emit_byte(OP_GREATER);
        break;
    case TOKEN_GREATER_EQUAL:
        expression_parser_emit_bytes(OP_LESS, OP_NOT);
        break;
    case TOKEN_LESS:
        expression_parser_emit_byte(OP_LESS);
        break;
    case TOKEN_LESS_EQUAL:
        expression_parser_emit_bytes(OP_GREATER, OP_NOT);
        break;
    case TOKEN_PLUS:
        expression_parser_emit_byte(OP_ADD);
        break;
    case TOKEN_MINUS:
        expression_parser_emit_byte(OP_SUBTRACT);
        break;
    case TOKEN_STAR:
        expression_parser_emit_byte(OP_MULTIPLY);
        break;
    case TOKEN_SLASH:
        expression_parser_emit_byte(OP_DIVIDE);
        break;
    case TOKEN_MODULO:
        expression_parser_emit_byte(OP_MODULO);
        break;
    case TOKEN_STAR_STAR:
        expression_parser_emit_byte(OP_EXPONENT);
        break;
    default:
        return;
    }
}

static inline void expression_parser_binary_number(bool canAssign) {
    (void)canAssign;
    double value = strtol(PARSER.previous.start + 2, NULL, 2);
    expression_parser_emit_constant_value(NUMBER_VAL(value));
}

static inline void expression_parser_call(bool canAssign) {
    (void)canAssign;
    uint8_t argCount = expression_parser_argument_list();
    expression_parser_emit_bytes(OP_CALL, argCount);
}

void expression_parser_compile_dynamic_array(bool canAssign) {
    (void)canAssign;
    uint8_t argCount = expression_parser_dynamic_array_argument_list();
    expression_parser_emit_bytes(OP_ARRAY_LITERAL, argCount);
}

static uint8_t expression_parser_dynamic_array_argument_list(void) {
    uint8_t argCount = 0;
    if (!expression_parser_check_token(TOKEN_RIGHT_BRACE)) {
        do {
            if (argCount == 255) {
                expression_parser_advance_token();
                expression_parser_error("Can't have more than 255 arguments in a array literal expression.");
            }
            expression_parser_parse_expression();
            argCount++;
        } while (expression_parser_match_token(TOKEN_COMMA));
    }
    expression_parser_consume_token(TOKEN_RIGHT_BRACE, "Expect '}' after arguments.");
    return argCount;
}

static void expression_parser_dot(bool canAssign) {
    expression_parser_consume_token(TOKEN_IDENTIFIER, "Expect property name after '.'.");
    uint8_t name = expression_parser_identifier_constant(&PARSER.previous);

    if (canAssign && expression_parser_match_token(TOKEN_EQUAL)) {
        expression_parser_parse_expression();
        expression_parser_emit_bytes(OP_SET_PROPERTY, name);
    } else if (canAssign && expression_parser_match_token(TOKEN_PLUS_EQUAL)) {
        expression_parser_nondirect_property_assignment(OP_ADD, name);
    } else if (canAssign && expression_parser_match_token(TOKEN_MINUS_EQUAL)) {
        expression_parser_nondirect_property_assignment(OP_SUBTRACT, name);
    } else if (canAssign && expression_parser_match_token(TOKEN_STAR_EQUAL)) {
        expression_parser_nondirect_property_assignment(OP_MULTIPLY, name);
    } else if (canAssign && expression_parser_match_token(TOKEN_SLASH_EQUAL)) {
        expression_parser_nondirect_property_assignment(OP_DIVIDE, name);
    } else if (canAssign && expression_parser_match_token(TOKEN_MODULO_EQUAL)) {
        expression_parser_nondirect_property_assignment(OP_MODULO, name);
    } else if (canAssign && expression_parser_match_token(TOKEN_STAR_STAR_EQUAL)) {
        expression_parser_nondirect_property_assignment(OP_EXPONENT, name);
    } else if (expression_parser_match_token(TOKEN_LEFT_PAREN)) {
        uint8_t argCount = expression_parser_argument_list();
        expression_parser_emit_bytes(OP_INVOKE, name);
        expression_parser_emit_byte(argCount);
    } else {
        expression_parser_emit_bytes(OP_GET_PROPERTY, name);
    }
}

void expression_parser_parse_expression(void) {
    expression_parser_parse_precedence(PREC_ASSIGNMENT);
}

static inline parse_rule_t * expression_parser_get_rule(tokentype type) {
    return &rules[type];
}

static void expression_parser_grouping(bool canAssign) {
    (void)canAssign;
    expression_parser_parse_expression();
    expression_parser_consume_token(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static inline void expression_parser_hex_number(bool canAssign) {
    (void)canAssign;
    double value = strtol(PARSER.previous.start + 2, NULL, 16);
    expression_parser_emit_constant_value(NUMBER_VAL(value));
}

uint8_t expression_parser_identifier_constant(token_t * name) {
    return expression_parser_make_constant(OBJECT_VAL(object_copy_string(name->start, name->length, false)));
}

bool expression_parser_identifiers_equal(token_t * a, token_t * b) {
    if (a->length != b->length) {
        return false;
    }
    return !memcmp(a->start, b->start, a->length);
}

static void expression_parser_index_of(bool canAssign, uint8_t getOp, uint32_t arg) {
    expression_parser_emit_bytes(getOp, (uint8_t)arg);
    expression_parser_parse_expression();
    if (expression_parser_match_token(TOKEN_RANGE)) {
        expression_parser_parse_expression();
        if (!expression_parser_match_token(TOKEN_RIGHT_BRACKET)) {
            expression_parser_error("expected closing bracket ]");
            return;
        }
        expression_parser_emit_byte(OP_GET_SLICE_OF);
        return;
    }
    if (!expression_parser_match_token(TOKEN_RIGHT_BRACKET)) {
        expression_parser_error("expected closing bracket ]");
        return;
    }
    if (expression_parser_match_token(TOKEN_EQUAL)) {
        if (!canAssign) {
            expression_parser_error("Invalid assignment target.");
        }
        expression_parser_parse_expression();
        expression_parser_emit_byte(OP_SET_INDEX_OF);
    } else {
        expression_parser_emit_byte(OP_GET_INDEX_OF);
    }
}

static void expression_parser_literal(bool canAssign) {
    (void)canAssign;
    switch (PARSER.previous.type) {
    case TOKEN_FALSE:
        expression_parser_emit_byte(OP_FALSE);
        break;
    case TOKEN_NULL:
        expression_parser_emit_byte(OP_NULL);
        break;
    case TOKEN_TRUE:
        expression_parser_emit_byte(OP_TRUE);
        break;
    default:
        return;
    }
}

void expression_parser_compile_named_variable(token_t name, bool canAssign) {
    uint8_t getOp, setOp;
    int32_t arg = expression_parser_resolve_local(CURRENT, &name);
    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else if ((arg = expression_parser_resolve_upvalue(CURRENT, &name)) != -1) {
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    } else {
        arg = expression_parser_identifier_constant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }
    if (canAssign && expression_parser_match_token(TOKEN_EQUAL)) {
        expression_parser_parse_expression();
        expression_parser_emit_bytes(setOp, (uint8_t)arg);
    } else if (canAssign && expression_parser_match_token(TOKEN_PLUS_EQUAL)) {
        expression_parser_nondirect_assignment(OP_ADD, getOp, setOp, arg);
    } else if (canAssign && expression_parser_match_token(TOKEN_MINUS_EQUAL)) {
        expression_parser_nondirect_assignment(OP_SUBTRACT, getOp, setOp, arg);
    } else if (canAssign && expression_parser_match_token(TOKEN_STAR_EQUAL)) {
        expression_parser_nondirect_assignment(OP_MULTIPLY, getOp, setOp, arg);
    } else if (canAssign && expression_parser_match_token(TOKEN_SLASH_EQUAL)) {
        expression_parser_nondirect_assignment(OP_DIVIDE, getOp, setOp, arg);
    } else if (canAssign && expression_parser_match_token(TOKEN_MODULO_EQUAL)) {
        expression_parser_nondirect_assignment(OP_MODULO, getOp, setOp, arg);
    } else if (canAssign && expression_parser_match_token(TOKEN_STAR_STAR_EQUAL)) {
        expression_parser_nondirect_assignment(OP_EXPONENT, getOp, setOp, arg);
    } else if (expression_parser_match_token(TOKEN_LEFT_BRACKET)) {
        expression_parser_index_of(canAssign, getOp, arg);
    } else {
        expression_parser_emit_bytes(getOp, (uint8_t)arg);
    }
}

static void expression_parser_nondirect_assignment(uint8_t assignmentType, uint8_t getOp, uint8_t setOp, uint8_t arg) {
    expression_parser_emit_bytes(getOp, arg);
    expression_parser_parse_expression();
    expression_parser_emit_byte(assignmentType);
    expression_parser_emit_bytes(setOp, arg);
}

// Compound assignment for object properties (e.g. this.count += 1).
// When called the receiver is already on the stack top.
// Stack sequence:
//   before:  ... receiver
//   DUP      ... receiver receiver
//   GET_PROP ... receiver old_value
//   <rhs>    ... receiver old_value rhs
//   <op>     ... receiver new_value
//   SET_PROP ... new_value              (SET_PROP pops receiver + value, pushes value back)
static void expression_parser_nondirect_property_assignment(uint8_t assignmentType, uint8_t name) {
    expression_parser_emit_byte(OP_DUP);
    expression_parser_emit_bytes(OP_GET_PROPERTY, name);
    expression_parser_parse_expression();
    expression_parser_emit_byte(assignmentType);
    expression_parser_emit_bytes(OP_SET_PROPERTY, name);
}

static inline void expression_parser_number(bool canAssign) {
    (void)canAssign;
    double value = strtod(PARSER.previous.start, NULL);
    expression_parser_emit_constant_value(NUMBER_VAL(value));
}

static void expression_parser_or(bool canAssign) {
    (void)canAssign;
    int32_t elseJump = expression_parser_emit_jump_offset(OP_JUMP_IF_FALSE);
    int32_t endJump = expression_parser_emit_jump_offset(OP_JUMP);
    expression_parser_patch_jump_offset(elseJump);
    expression_parser_emit_byte(OP_POP);
    expression_parser_parse_precedence(PREC_OR);
    expression_parser_patch_jump_offset(endJump);
}

static void expression_parser_parse_precedence(precedence precedence) {
    expression_parser_advance_token();
    parse_function_t prefixRule = expression_parser_get_rule(PARSER.previous.type)->prefix;
    if (!prefixRule) {
        expression_parser_error("Expect expression.");
        return;
    }
    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);
    while (precedence <= expression_parser_get_rule(PARSER.current.type)->precedence) {
        expression_parser_advance_token();
        parse_function_t infixRule = expression_parser_get_rule(PARSER.previous.type)->infix;
        if (infixRule) {
            infixRule(canAssign);
        } else {
            expression_parser_error("Invalid Token at the current position");
        }
    }
    if (canAssign && expression_parser_match_token(TOKEN_EQUAL)) {
        expression_parser_error("Invalid assignment target.");
    }
}

static int32_t expression_parser_resolve_local(compiler_t * compiler, token_t * name) {
    for (int32_t i = compiler->localCount - 1; i >= 0; i--) {
        local_t * local = &compiler->locals[i];
        if (expression_parser_identifiers_equal(name, &local->name)) {
            if (local->depth == -1) {
                expression_parser_error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }
    return -1;
}

static int32_t expression_parser_resolve_upvalue(compiler_t * compiler, token_t * name) {
    if (!compiler->enclosing) {
        return -1;
    }
    int32_t local = expression_parser_resolve_local(compiler->enclosing, name);
    if (local != -1) {
        compiler->enclosing->locals[local].isCaptured = true;
        return expression_parser_add_upvalue(compiler, (uint8_t)local, true);
    }
    int32_t upvalue = expression_parser_resolve_upvalue(compiler->enclosing, name);
    if (upvalue != -1) {
        return expression_parser_add_upvalue(compiler, (uint8_t)upvalue, false);
    }
    return -1;
}

static void expression_parser_string(bool canAssign) {
    (void)canAssign;
    object_string_t * string = object_copy_string(PARSER.previous.start + 1, PARSER.previous.length - 2, true);
    if (!string) {
        expression_parser_error("Unknown escape sequence in string");
        return;
    }
    expression_parser_emit_constant_value(OBJECT_VAL(string));
}

static void expression_parser_super(bool canAssign) {
    (void)canAssign;
    if (!CURRENT_CLASS) {
        expression_parser_error("Can't use 'super' outside of a class.");
    } else if (!CURRENT_CLASS->hasSuperclass) {
        expression_parser_error("Can't use 'super' in a class with no superclass.");
    }
    expression_parser_consume_token(TOKEN_DOT, "Expect '.' after 'super'.");
    expression_parser_consume_token(TOKEN_IDENTIFIER, "Expect superclass method name.");
    uint8_t name = expression_parser_identifier_constant(&PARSER.previous);
    expression_parser_compile_named_variable(expression_parser_make_synthetic_token("this"), false);
    if (expression_parser_match_token(TOKEN_LEFT_PAREN)) {
        uint8_t argCount = expression_parser_argument_list();
        expression_parser_compile_named_variable(expression_parser_make_synthetic_token("super"), false);
        expression_parser_emit_bytes(OP_SUPER_INVOKE, name);
        expression_parser_emit_byte(argCount);
    } else {
        expression_parser_compile_named_variable(expression_parser_make_synthetic_token("super"), false);
        expression_parser_emit_bytes(OP_GET_SUPER, name);
    }
}

static void expression_parser_this(bool canAssign) {
    (void)canAssign;
    if (!CURRENT_CLASS) {
        expression_parser_error("Can't use 'this' outside of a class.");
        return;
    }
    expression_parser_compile_variable(false);
}

static void expression_parser_try(bool canAssign) {
    (void)canAssign;
    expression_parser_parse_precedence(PREC_UNARY);
    expression_parser_emit_byte(OP_TRY_PROPAGATE);
}

static void expression_parser_must(bool canAssign) {
    (void)canAssign;
    expression_parser_parse_precedence(PREC_UNARY);
    expression_parser_emit_byte(OP_MUST);
}

static void expression_parser_parse_handler_block(void) {
    // Block handler evaluates to its last expression value.
    // Grammar shape: '{' expression (';' expression)* ';'? '}'
    expression_parser_consume_token(TOKEN_LEFT_BRACE, "Expect '{' to start handler block.");

    if (expression_parser_match_token(TOKEN_RIGHT_BRACE)) {
        expression_parser_emit_byte(OP_NULL);
        return;
    }

    expression_parser_parse_expression();
    while (expression_parser_match_token(TOKEN_SEMICOLON)) {
        if (expression_parser_check_token(TOKEN_RIGHT_BRACE)) {
            // Trailing ';' means no final expression remains.
            expression_parser_emit_byte(OP_POP);
            expression_parser_emit_byte(OP_NULL);
            break;
        }
        expression_parser_emit_byte(OP_POP);
        expression_parser_parse_expression();
    }

    expression_parser_consume_token(TOKEN_RIGHT_BRACE, "Expect '}' after handler block.");
}

static void expression_parser_parse_pipe_bound_handler(precedence handlerPrecedence) {
    expression_parser_consume_token(TOKEN_PIPE, "Expect '|' before bound variable name.");
    expression_parser_consume_token(TOKEN_IDENTIFIER, "Expect bound variable name.");
    token_t boundName = PARSER.previous;
    expression_parser_consume_token(TOKEN_PIPE, "Expect '|' after bound variable name.");

    compiler_begin_scope();
    compiler_add_local(boundName);
    compiler_mark_initialized();

    uint8_t boundSlot = (uint8_t)(CURRENT->localCount - 1);
    if (expression_parser_check_token(TOKEN_LEFT_BRACE)) {
        expression_parser_parse_handler_block();
    } else {
        expression_parser_parse_precedence(handlerPrecedence);
    }

    // Move the handler result into the binder's stack slot, then drop the temporary top value.
    // After metadata unwind below, that slot remains as the expression result.
    expression_parser_emit_bytes(OP_SET_LOCAL, boundSlot);
    expression_parser_emit_byte(OP_POP);

    if (CURRENT->locals[boundSlot].isCaptured) {
        expression_parser_emit_byte(OP_CLOSE_UPVALUE_KEEP);
    }

    CURRENT->scopeDepth--;
    CURRENT->localCount--;
}

static void expression_parser_iferror_expression(bool canAssign) {
    (void)canAssign;
    expression_parser_parse_expression();
    expression_parser_emit_byte(OP_RESULT_IS_ERROR);
    int32_t elseBranch = expression_parser_emit_jump_offset(OP_JUMP_IF_FALSE);

    // error branch
    expression_parser_emit_byte(OP_POP);
    expression_parser_emit_byte(OP_RESULT_UNWRAP_ERROR);
    expression_parser_parse_pipe_bound_handler(PREC_ASSIGNMENT);
    int32_t endJump = expression_parser_emit_jump_offset(OP_JUMP);

    // success branch
    expression_parser_patch_jump_offset(elseBranch);
    expression_parser_emit_byte(OP_POP);
    expression_parser_emit_byte(OP_RESULT_UNWRAP);
    expression_parser_consume_token(TOKEN_ELSE, "Expect 'else' in iferror expression.");
    expression_parser_parse_pipe_bound_handler(PREC_ASSIGNMENT);

    expression_parser_patch_jump_offset(endJump);
}

// infix: lhs (a result value) is already on stack
static void expression_parser_catch(bool canAssign) {
    (void)canAssign;
    // Stack: [..., result]
    expression_parser_emit_byte(OP_RESULT_IS_ERROR); // [..., result, bool]
    int32_t successJump = expression_parser_emit_jump_offset(OP_JUMP_IF_FALSE);
    // error path
    expression_parser_emit_byte(OP_POP); // pop true
    bool hasSuccessHandler = false;
    if (expression_parser_check_token(TOKEN_PIPE)) {
        expression_parser_emit_byte(OP_RESULT_UNWRAP_ERROR); // pop result, push error payload
        expression_parser_parse_pipe_bound_handler(PREC_OR);
        if (expression_parser_match_token(TOKEN_ELSE)) {
            hasSuccessHandler = true;
        }
    } else {
        expression_parser_emit_byte(OP_POP);         // pop error result
        expression_parser_parse_precedence(PREC_OR); // evaluate fallback
    }

    int32_t endJump = expression_parser_emit_jump_offset(OP_JUMP);

    // success path
    expression_parser_patch_jump_offset(successJump);
    expression_parser_emit_byte(OP_POP); // pop false
    expression_parser_emit_byte(OP_RESULT_UNWRAP);

    if (hasSuccessHandler) {
        expression_parser_parse_pipe_bound_handler(PREC_OR);
    }

    expression_parser_patch_jump_offset(endJump);
}

static void expression_parser_unary(bool canAssign) {
    (void)canAssign;
    tokentype operatorType = PARSER.previous.type;
    expression_parser_parse_precedence(PREC_UNARY);
    switch (operatorType) {
    case TOKEN_BANG:
        expression_parser_emit_byte(OP_NOT);
        break;
    case TOKEN_MINUS:
        expression_parser_emit_byte(OP_NEGATE);
        break;
    default:
        return;
    }
}

void expression_parser_compile_variable(bool canAssign) {
    expression_parser_compile_named_variable(PARSER.previous, canAssign);
}
