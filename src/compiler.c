#include "compiler.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
// The debug header file only needs to be included if the bytecode is dissasembled
#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif
#include "lexer.h"
#include "memory.h"
#include "virtual_machine.h"

/// @brief The cellox parser
/// @details The parser builds an abstract syntax tree out of the tokens that were produced by the lexer
typedef struct
{
    /// The token that is currently being parsed
    token_t current;
    /// The token that was previously parsed
    token_t previous;
    /// Flag that indicates whether an error occured during the compilation
    bool hadError;
    /// Flag that indicates that the compiler couldn't synchronize after an errror occured
    bool panicMode;
}parser_t;

/// @brief Precedences that corresponds to a single or a group of tokens
typedef enum
{
    /// Lowest precedence (literals)
    PREC_NONE,
    ///  Precedence of &quot;= += -= *= /= %= **=&quot;
    PREC_ASSIGNMENT,
    ///  Precedence of &quot;or ||&quot;
    PREC_OR,
    /// Precedence of "and  &&"
    PREC_AND,
    /// Precedence of &quot;== !=&quot;
    PREC_EQUALITY,
    /// Precedence of &quot;< > <= >=&quot;
    PREC_COMPARISON,
    /// Precedence of &quot;\+ &minus;&quot;
    PREC_TERM,
    /// Precedence of &quot;\* / % **&quot;
    PREC_FACTOR,
    /// Precedence of &quot;! -&quot;
    PREC_UNARY,
    ///  &quot;. () []&quot;
    PREC_CALL,
    /// Primary precedence (unused)
    PREC_PRIMARY
} precedence_t;

/// @brief A parse function
/// @details This provides a common pattern for all parsing function
typedef void (* parse_function_t)(bool canAssign);

/// @brief A parsing rule that applies to a special token
typedef struct
{
    /// @brief The prefix rule of the parsing rule
    parse_function_t prefix;
    /// @brief The infix rule of the parsing rule
    parse_function_t infix;
    /// @brief The precedence of a token
    precedence_t precedence;
} parse_rule_t;

/// @brief A local variable structure
typedef struct
{
    /// Name of the local variable
    token_t name;
    /// Scope depth where the local variable was declared
    int32_t depth;
    /// Boolean value that determines whether the local variable is captured by a closure
    bool isCaptured;
} local_t;

/// @brief An upvalue structure
typedef struct
{
    /// Index of the upvalue
    uint8_t index;
    /// Flag that indicates whether the value is a local value
    bool isLocal;
} upvalue_t;

/// @brief A cellox function 
typedef enum
{
    /// Marks a normal function
    TYPE_FUNCTION,
    /// @brief Marks an initializer function
    /// @details In other programming languages this often called a contstuctor
    TYPE_INITIALIZER,
    /// A method that is bound to a class
    TYPE_METHOD,
    /// A cellox script
    TYPE_SCRIPT
} function_type_t;

/// @brief The cellox compiler
typedef struct compiler_t
{
    /// @brief The enclosing compiler
    /// @details This is needed to compile functions that are enclosed in another function or a script
    struct compiler_t * enclosing;
    /// @brief The main function
    object_function_t * function;
    /// @brief The type of the function that is currently executed
    function_type_t type;
    /// @brief The locals that were declared in the current scope
    local_t locals[UINT8_COUNT];
    // @brief The amount of local values that where declared in the current scope
    int32_t localCount;
    /// @brief The upvalues of the current scope (part of a closure)
    upvalue_t upvalues[UINT8_COUNT];
    /// @brief The scopedepth
    /// @details Used to determine whether a declared variable is a global or a local variable
    int32_t scopeDepth;
} compiler_t;

/// @brief  Classcompiler struct definition
typedef struct class_compiler_t
{
    /// The enclosing class compiler structure
    struct class_compiler_t * enclosing;
    /// @brief boolean value that determines whether a class has a superclass
    bool hasSuperclass;
} class_compiler_t;

/// @brief Global parser variable
parser_t parser;

/// @brief Global compiler variable
compiler_t * current = NULL;

/// @brief Global classCompiler variable
/// @details Used to model inheritance for a cellox class
class_compiler_t * currentClass = NULL;

static void compiler_add_local(token_t);
static uint32_t compiler_add_upvalue(compiler_t *, uint8_t, bool);
static void compiler_advance();
static void compiler_and(bool);
static uint8_t compiler_argument_list();
static void compiler_begin_scope();
static void compiler_binary(bool);
static void compiler_block();
static void compiler_call(bool);
static bool compiler_check(tokentype_t);
static void compiler_class_declaration();
static void compiler_consume(tokentype_t, char const *);
static chunk_t *compiler_current_chunk();
static void compiler_declaration();
static void compiler_declare_variable();
static void compiler_define_variable(uint8_t);
static void compiler_dot(bool);
static void compiler_emit_byte(uint8_t);
static void compiler_emit_bytes(uint8_t, uint8_t);
static void compiler_emit_constant(value_t);
static int32_t compiler_emit_jump(uint8_t);
static void compiler_emit_loop(int32_t);
static void compiler_emit_return();
static object_function_t *compiler_end();
static void compiler_end_scope();
static void compiler_error(char const *, ...);
static void compiler_error_at(token_t *, char const *, ...);
static void compiler_error_at_current(char const *, ...);
static void compiler_expression();
static void compiler_expression_statement();
static void compiler_for_statement();
static void compiler_function(function_type_t);
static void compiler_function_declaration();
static parse_rule_t * compiler_get_rule(tokentype_t);
static void compiler_grouping(bool);
static uint8_t compiler_identifier_constant(token_t *);
static bool compiler_identifiers_equal(token_t *, token_t *);
static void compiler_if_statement();
static void compiler_init(compiler_t *, function_type_t);
static void compiler_index_of(bool, uint8_t, uint8_t, uint32_t);
static void compiler_literal(bool);
static void compiler_mark_initialized();
static uint8_t compiler_make_constant(value_t);
static bool compiler_match_token(tokentype_t);
static void compiler_method();
static void compiler_named_variable(token_t, bool);
static void compiler_nondirect_assignment(uint8_t, uint8_t, uint8_t, uint8_t);
static void compiler_number(bool);
static void compiler_or(bool);
static void compiler_parse_precedence(precedence_t);
static uint8_t compiler_parse_variable(char const *);
static void compiler_patch_jump(int32_t);
static int32_t compiler_resolve_local(compiler_t *, token_t *);
static int32_t compiler_resolve_upvalue(compiler_t *, token_t *);
static void compiler_return_statement();
static void compiler_statement();
static void compiler_string(bool);
static void compiler_super(bool);
static void compiler_synchronize();
static token_t compiler_synthetic_token(char const *);
static void compiler_this(bool);
static void compiler_unary(bool);
static void compiler_var_declaration();
static void compiler_variable(bool);
static void compiler_while_statement();

/// ParseRules for the tokens of cellox
static parse_rule_t rules[] = 
{
    [TOKEN_AND] =
    {
        .prefix = NULL, 
        .infix = compiler_and, 
        .precedence = PREC_AND
    },
    [TOKEN_BANG] =
    { 
        .prefix = compiler_unary, 
        .infix = NULL, 
        .precedence = PREC_UNARY
    },
    [TOKEN_BANG_EQUAL] =
    {
        .prefix = NULL, 
        .infix = compiler_binary, 
        .precedence = PREC_EQUALITY
    },
    [TOKEN_CLASS] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_COMMA] =
    { 
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_DOT] =
    {
        .prefix = NULL, 
        .infix = compiler_dot, 
        .precedence = PREC_CALL
    },
    [TOKEN_EOF] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_ELSE] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_EQUAL] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_ASSIGNMENT
    },
    [TOKEN_EQUAL_EQUAL] =
    {
        .prefix = NULL, 
        .infix = compiler_binary, 
        .precedence = PREC_EQUALITY
    },
    [TOKEN_ERROR] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_FALSE] =
    {
        .prefix = compiler_literal, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_FOR] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_FUN] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_GREATER] =
    {
        .prefix = NULL, 
        .infix = compiler_binary, 
        .precedence = PREC_COMPARISON
    },
    [TOKEN_GREATER_EQUAL] =
    {
        .prefix = NULL, 
        .infix = compiler_binary, 
        .precedence = PREC_COMPARISON
    },
    [TOKEN_IDENTIFIER] =
    {
        .prefix = compiler_variable, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_IF] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_LEFT_BRACE] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_LEFT_PAREN] =
    {
        .prefix = compiler_grouping, 
        .infix = compiler_call, 
        .precedence = PREC_CALL
    },
    [TOKEN_LEFT_BRACKET] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_CALL
    },
    [TOKEN_LESS] =
    {
        .prefix = NULL, 
        .infix = compiler_binary, 
        .precedence = PREC_COMPARISON
    },
    [TOKEN_LESS_EQUAL] =
    {
        .prefix = NULL, 
        .infix = compiler_binary, 
        .precedence = PREC_COMPARISON
    },
    [TOKEN_MODULO] =
    {
        .prefix = NULL, 
        .infix = compiler_binary, 
        .precedence = PREC_FACTOR
    },
    [TOKEN_MODULO_EQUAL] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_ASSIGNMENT
    },
    [TOKEN_MINUS] =
    {
        .prefix = compiler_unary, 
        .infix = compiler_binary, 
        .precedence = PREC_TERM
    },
    [TOKEN_MINUS_EQUAL] =
    {
        .prefix = NULL,
        .infix = NULL,
        .precedence = PREC_ASSIGNMENT
    },
    [TOKEN_NULL] =
    {
        .prefix = compiler_literal, 
        .infix = NULL,
        .precedence = PREC_NONE
    },
    [TOKEN_NUMBER] =
    {
        .prefix = compiler_number,
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_PLUS] =
    {
        .prefix = NULL, 
        .infix = compiler_binary, 
        .precedence = PREC_TERM
    },
    [TOKEN_PLUS_EQUAL] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_ASSIGNMENT
    },
    [TOKEN_OR] =
    {
        .prefix = NULL, 
        .infix = compiler_or, 
        .precedence = PREC_OR
    },
    [TOKEN_PRINT] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_RETURN] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_RIGHT_BRACE] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_RIGHT_PAREN] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_RIGHT_BRACKET] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_SEMICOLON] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_SLASH] =
    {
        .prefix = NULL, 
        .infix = compiler_binary, 
        .precedence = PREC_FACTOR
    },
    [TOKEN_SLASH_EQUAL] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_ASSIGNMENT
    },
    [TOKEN_STAR] =
    {
        .prefix = NULL, 
        .infix = compiler_binary, 
        .precedence = PREC_FACTOR
    },
    [TOKEN_STAR_EQUAL] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_ASSIGNMENT
    },
    [TOKEN_STAR_STAR] =
    {
        .prefix = NULL, 
        .infix = compiler_binary, 
        .precedence = PREC_FACTOR
    },
    [TOKEN_STRING] =
    {
        .prefix = compiler_string, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_SUPER] =
    {
        .prefix = compiler_super, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_THIS] =
    {
        .prefix = compiler_this, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_TRUE] =
    {
        .prefix = compiler_literal, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_VAR] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    },
    [TOKEN_WHILE] =
    {
        .prefix = NULL, 
        .infix = NULL, 
        .precedence = PREC_NONE
    }
};

object_function_t * compiler_compile(char const * program)
{
    lexer_init(program);
    compiler_t compiler;
    compiler_init(&compiler, TYPE_SCRIPT);
    parser.hadError = false;
    parser.panicMode = false;
    compiler_advance();
    // We keep compiling until we hit the end of the source file
    while (!compiler_match_token(TOKEN_EOF))
        compiler_declaration();
    object_function_t *function = compiler_end();
    return parser.hadError ? NULL : function;
}

void compiler_mark_roots()
{
    compiler_t * compiler = current;
    while (compiler)
    {
        memory_mark_object((object_t *)compiler->function);
        compiler = compiler->enclosing;
    }
}

/// Adds a new local variable to the stack
static void compiler_add_local(token_t name)
{
    if (current->localCount == UINT8_COUNT)
    {
        /// We can only have 255 local variable on the stack 😔
        compiler_error("Too many local variables in function.");
        return;
    }
    local_t * local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
    local->isCaptured = false;
}

/// @brief Adds an upValue to the compiler
/// @param compiler The compiler where the upvalue is added
/// @param index The index of the upvalue
/// @param isLocal A boolean value that indicates whether the upvalue comes from a local variable
/// @return The index of the upvalue
static uint32_t compiler_add_upvalue(compiler_t * compiler, uint8_t index, bool isLocal)
{
    uint32_t upvalueCount = compiler->function->upvalueCount;

    for (uint32_t i = 0; i < upvalueCount; i++)
    {
        upvalue_t * upvalue = &compiler->upvalues[i];
        if (upvalue->index == index && upvalue->isLocal == isLocal)
            return i;
    }
    if (upvalueCount == UINT8_COUNT)
    {
        compiler_error("Too many closure variables in function.");
        return 0;
    }

    compiler->upvalues[upvalueCount].isLocal = isLocal;
    compiler->upvalues[upvalueCount].index = index;
    return compiler->function->upvalueCount++;
}

/// @brief Advances a poosition further in the linear sequence of tokens
static void compiler_advance()
{
    parser.previous = parser.current;

    for (;;)
    {
        parser.current = scan_token();
        if (parser.current.type != TOKEN_ERROR)
            break;

        compiler_error_at_current(parser.current.start);
    }
}

/// @brief Parses an and expression
/// @param canAssign Unused for and expreesion
static void compiler_and(bool canAssign)
{
    int32_t endJump = compiler_emit_jump(OP_JUMP_IF_FALSE);
    compiler_emit_byte(OP_POP);
    compiler_parse_precedence(PREC_AND);
    compiler_patch_jump(endJump);
}

/// @brief Compiles a List of argument of a call expression to bytecode instructions
/// @return The amount of arguments that were parsed
static uint8_t compiler_argument_list()
{
    uint8_t argCount = 0;
    if (!compiler_check(TOKEN_RIGHT_PAREN))
    {
        do
        {
            compiler_expression();
            if (argCount == 255)
                compiler_error("Can't have more than 254 arguments.");
            argCount++;
        } while (compiler_match_token(TOKEN_COMMA));
    }
    compiler_consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
    return argCount;
}

/// @brief Handles the beginning of a new Scope
/// @details Increments the scopecounter that is used to determine whether the current scope is global &frasl; local
static void compiler_begin_scope()
{
    current->scopeDepth++;
}

/// @brief Compiles a binary expression
/// @param canAssign Unused for binary expressions
static void compiler_binary(bool canAssign)
{
    tokentype_t operatorType = parser.previous.type;
    parse_rule_t * rule = compiler_get_rule(operatorType);
    compiler_parse_precedence((precedence_t)(rule->precedence + 1));

    switch (operatorType)
    {
    case TOKEN_BANG_EQUAL:
        compiler_emit_bytes(OP_EQUAL, OP_NOT);
        break;
    case TOKEN_EQUAL_EQUAL:
        compiler_emit_byte(OP_EQUAL);
        break;
    case TOKEN_GREATER:
        compiler_emit_byte(OP_GREATER);
        break;
    case TOKEN_GREATER_EQUAL:
        compiler_emit_bytes(OP_LESS, OP_NOT);
        break;
    case TOKEN_LESS:
        compiler_emit_byte(OP_LESS);
        break;
    case TOKEN_LESS_EQUAL:
        compiler_emit_bytes(OP_GREATER, OP_NOT);
        break;
    case TOKEN_PLUS:
        compiler_emit_byte(OP_ADD);
        break;
    case TOKEN_MINUS:
        compiler_emit_byte(OP_SUBTRACT);
        break;
    case TOKEN_STAR:
        compiler_emit_byte(OP_MULTIPLY);
        break;
    case TOKEN_SLASH:
        compiler_emit_byte(OP_DIVIDE);
        break;
    case TOKEN_MODULO:
        compiler_emit_byte(OP_MODULO);
        break;
    case TOKEN_STAR_STAR:
        compiler_emit_byte(OP_EXPONENT);
        break;
    default:
        return;
    }
}

/// @brief Compiles a block statement to bytecode instructions
static void compiler_block()
{
    while (!compiler_check(TOKEN_RIGHT_BRACE) && !compiler_check(TOKEN_EOF))
        compiler_declaration();
    compiler_consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

/// @brief Compiles a call expression
/// @param canAssign Unused for call expressions
/// @details For that purpose all the arguments when calling the function are compiled and a CALL instruction is emited 
/// followed by the amount of arguments, that where used when the function was called
static void compiler_call(bool canAssign)
{
    uint8_t argCount = compiler_argument_list();
    compiler_emit_bytes(OP_CALL, argCount);
}

/// @brief  Checks if the next Token is of a given type
/// @param type The type that the token is matched against
/// @return True if the next token matches the type, false if not
/// @details We do not advance if the token is of the specified type
static bool compiler_check(tokentype_t type)
{
    return parser.current.type == type;
}

/// @brief Parses a new class declaration
static void compiler_class_declaration()
{
    // Compiles the name of the class
    compiler_consume(TOKEN_IDENTIFIER, "Expect class name.");
    token_t className = parser.previous;
    uint8_t nameConstant = compiler_identifier_constant(&parser.previous);
    compiler_declare_variable();
    compiler_emit_bytes(OP_CLASS, nameConstant);
    compiler_define_variable(nameConstant);
    class_compiler_t classCompiler;
    // If this class has a superclass we will set this to true later
    classCompiler.hasSuperclass = false;
    classCompiler.enclosing = currentClass;
    currentClass = &classCompiler;
    if (compiler_match_token(TOKEN_DOUBLEDOT))
    {
        compiler_consume(TOKEN_IDENTIFIER, "Expect superclass name.");
        compiler_variable(false);
        if (compiler_identifiers_equal(&className, &parser.previous))
            compiler_error("A class can't inherit from itself.");
        compiler_begin_scope();
        compiler_add_local(compiler_synthetic_token("super"));
        compiler_define_variable(0);
        compiler_named_variable(className, false);
        compiler_emit_byte(OP_INHERIT);
        // The class has a super class declared
        classCompiler.hasSuperclass = true;
    }
    compiler_named_variable(className, false);
    compiler_consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");
    while (!compiler_check(TOKEN_RIGHT_BRACE) && !compiler_check(TOKEN_EOF))
        compiler_method();
    compiler_consume(TOKEN_RIGHT_BRACE, "Expect '}' after class body.");
    compiler_emit_byte(OP_POP);
    if (classCompiler.hasSuperclass)
        compiler_end_scope();
    currentClass = currentClass->enclosing;
}

/// @brief Consumes a Token and emits a error message if the type of the token doesn't match the specified type
/// @param type The expected type of the token
/// @param message The error message that is displayed, if the compiler is not of the specified type
/// @details The program will exit with an exit code that indicates an error at compile time
static void compiler_consume(tokentype_t type, char const * message)
{
    if (parser.current.type == type)
    {
        compiler_advance();
        return;
    }
    compiler_error_at_current(message);
}

/// @brief Gets the token that is currently compiled
/// @details Gets a pointer the the token in the chunk that is currently compiled
static chunk_t * compiler_current_chunk()
{
    return &current->function->chunk;
}

/// @brief Compiles a declaration stament or another statement
/// @details A declaration statement can be a class-, function- or variable declaration.
/// If the next statement is not a declaration it is instead compiled (precedence)
static void compiler_declaration()
{
    if (compiler_match_token(TOKEN_CLASS))
        compiler_class_declaration();
    else if (compiler_match_token(TOKEN_FUN))
        compiler_function_declaration();
    else if (compiler_match_token(TOKEN_VAR))
        compiler_var_declaration();
    else
        compiler_statement();
    if (parser.panicMode)
        compiler_synchronize();
}

/// @brief Compiles the declaration of a variable
/// @details Checks for local varoables, if there already has been declared a variable with that name at the current scope
static void compiler_declare_variable()
{
    // Global scope
    if (!current->scopeDepth)
        return;

    token_t * name = &parser.previous;
    for (int32_t i = current->localCount - 1; i >= 0; i--)
    {
        local_t *local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scopeDepth)
            break;

        if (compiler_identifiers_equal(name, &local->name))
            compiler_error("Already a variable with this name in this scope.");
    }
    compiler_add_local(*name);
}

/// @brief Compiles the definition of a variable
/// @param global The slot of the value
static void compiler_define_variable(uint8_t global)
{
    if (current->scopeDepth > 0)
    {
        // Marks the variable as initialized (only used for local variables)
        compiler_mark_initialized();
        return;
    }
    // The variable has been declared at the global / top level scope
    compiler_emit_bytes(OP_DEFINE_GLOBAL, global);
}

/// @brief Compiles a dot statement
/// @param canAssign Boolean value that determines whether the value can be changed
/// @details This can either be a getting the value of field, setting the value of a field 
/// or invoking a method of a cellox object instance.
static void compiler_dot(bool canAssign)
{
    compiler_consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
    uint8_t name = compiler_identifier_constant(&parser.previous);

    if (canAssign && compiler_match_token(TOKEN_EQUAL))
    {
        compiler_expression();
        compiler_emit_bytes(OP_SET_PROPERTY, name);
    }
    else if (compiler_match_token(TOKEN_LEFT_PAREN))
    {
        uint8_t argCount = compiler_argument_list();
        compiler_emit_bytes(OP_INVOKE, name);
        compiler_emit_byte(argCount);
    }
    else
    {
        compiler_emit_bytes(OP_GET_PROPERTY, name);
    }
}

/// @brief Emits a single byte
/// @param byte The byte that is emitted
static void compiler_emit_byte(uint8_t byte)
{
    chunk_write(compiler_current_chunk(), byte, parser.previous.line);
}

/// @brief Emits two bytes
/// @param byte1 The first byte that is emitted
/// @param byte2 The second byte that is emmited
static void compiler_emit_bytes(uint8_t byte1, uint8_t byte2)
{
    compiler_emit_byte(byte1);
    compiler_emit_byte(byte2);
}

/// @brief Creates a constant bytecode instruction
/// @param value The value of the constant
/// This can either be a numerical value or a cellox object
static void compiler_emit_constant(value_t value)
{
    compiler_emit_bytes(OP_CONSTANT, compiler_make_constant(value));
}

/// @brief Emits a bytecode instruction of the type jump (jump or jump-if-false) and writes a placeholder to the jump offset
/// @param instruction The bytecode instruction that is emitted
/// @return offset (start address) of the then or else branch
static int32_t compiler_emit_jump(uint8_t instruction)
{
    compiler_emit_byte(instruction);
    compiler_emit_byte(0xff);
    compiler_emit_byte(0xff);
    return compiler_current_chunk()->count - 2;
}

/// Emits the bytecode instructions for creating a loop
static void compiler_emit_loop(int32_t loopStart)
{
    compiler_emit_byte(OP_LOOP);
    int32_t offset = compiler_current_chunk()->count - loopStart + 2;
    if (offset > (int32_t)UINT16_MAX)
        compiler_error("Loop body too large."); // There can only be 65535 lines betweem a jump instruction because we use a short
    compiler_emit_byte((offset >> 8) & 0xff);
    compiler_emit_byte(offset & 0xff);
}

/// @brief Emits a return bytecode instruction
static void compiler_emit_return()
{
    if (current->type == TYPE_INITIALIZER)
        compiler_emit_bytes(OP_GET_LOCAL, 0);
    else
        compiler_emit_byte(OP_NULL);
    compiler_emit_byte(OP_RETURN);
}

/// @brief Yields a newly created function object after the compilation process finished
static object_function_t * compiler_end()
{
    compiler_emit_return();
    object_function_t * function = current->function;
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError)
        debug_disassemble_chunk(compiler_current_chunk(), function->name != NULL ? function->name->chars : "main", function->arity);
#endif
    current = current->enclosing;
    return function;
}

/// @brief Handles the closing of a scope
/** @details We are looking for any variables,
 * that where declared at the scope depth we just left, when we pop a scope.
 * If the value is captured it is an upvalue and therefore the value needs to be adjusted.
*/
static void compiler_end_scope()
{
    current->scopeDepth--;
    // We walk backward through the local array looking for the upvalues in the variables
    while (current->localCount > 0 && current->locals[current->localCount - 1].depth > current->scopeDepth)
    {
        if (current->locals[current->localCount - 1].isCaptured)
            compiler_emit_byte(OP_CLOSE_UPVALUE);
        else
            compiler_emit_byte(OP_POP);
        current->localCount--;
    }
}

/// @brief Reports an error at the previous position
/// @param message The error message that is displayed
static void compiler_error(char const * format, ...)
{
    va_list args;
    va_start(args, format);
    compiler_error_at(&parser.previous, format, args);
}

/// @brief Reports an error that was triggered by a specifiec token
/// @param token The token that has produced the error
/// @param message The message that is omited by the compiler
static void compiler_error_at(token_t * token, char const * format, ...)
{
    if (parser.panicMode)
        return;
    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);
    if (token->type == TOKEN_EOF)
        fprintf(stderr, " at end: ");
    else
        fprintf(stderr, " at '%.*s': ", token->length, token->start);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
    parser.hadError = true;
}

/// @brief Reports an error at the current position
/// @param message The error message that is displayed
/// @details Exits the program with an exitcode that indicates a compilation error
static void compiler_error_at_current(char const * format, ...)
{
    va_list args;
    va_start(args, format);
    compiler_error_at(&parser.current, format);
}

/// @brief Compiles a expression
/// @details A Expression in cellox can be a assignment expression, a get expression, a set expression, or expressions, 
/// and expression, equality-expressions, comparison expression, term expression, factor expression, unary expression,
/// call expression, set index of expression, get index of expression, identifier expression, grouping expression, or literal expression
static void compiler_expression()
{
    compiler_parse_precedence(PREC_ASSIGNMENT);
}

/// @brief Compiles an expression-statement
static void compiler_expression_statement()
{
    compiler_expression();
    compiler_consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    compiler_emit_byte(OP_POP);
}

/// @brief Compiles a for-statement
static void compiler_for_statement()
{
    compiler_begin_scope();
    compiler_consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

    // Initializer clause
    if (compiler_match_token(TOKEN_VAR))
        compiler_var_declaration();
    else if(!compiler_match_token(TOKEN_SEMICOLON))
        compiler_expression_statement();

    int32_t loopStart = compiler_current_chunk()->count;
    int32_t exitJump = -1;

    // Conditional clause
    if (!compiler_match_token(TOKEN_SEMICOLON))
    {
        compiler_expression();
        compiler_consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");
        // Jump out of the loop if the condition is false.
        exitJump = compiler_emit_jump(OP_JUMP_IF_FALSE);
        compiler_emit_byte(OP_POP); // Condition.
    }

    // Increment clause
    if (!compiler_match_token(TOKEN_RIGHT_PAREN))
    {
        int32_t bodyJump = compiler_emit_jump(OP_JUMP);
        int32_t incrementStart = compiler_current_chunk()->count;
        compiler_expression();
        compiler_emit_byte(OP_POP);
        compiler_consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");
        compiler_emit_loop(loopStart);
        loopStart = incrementStart;
        compiler_patch_jump(bodyJump);
    }

    compiler_statement();
    compiler_emit_loop(loopStart);

    if (exitJump != -1)
    {
        compiler_patch_jump(exitJump);
        compiler_emit_byte(OP_POP); // Condition.
    }

    compiler_end_scope();
}

/// @brief Compiles a function declaration statement to bytecode instructions
/// @param type The type of the function that is compiled
static void compiler_function(function_type_t type)
{
    compiler_t compiler;
    compiler_init(&compiler, type);
    compiler_begin_scope();

    compiler_consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");
    if (!compiler_check(TOKEN_RIGHT_PAREN))
    {
        // Compiling parameters of the function declaration
        do
        {
            // We expect an additional argument when the function is called
            current->function->arity++;
            // There are too much parameters specified
            if (current->function->arity > 255)
                compiler_error_at_current("Can't have more than 255 parameters.");
            uint8_t constant = compiler_parse_variable("Expect parameter name.");
            compiler_define_variable(constant);
        } while (compiler_match_token(TOKEN_COMMA));
    }
    compiler_consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    compiler_consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    // Compiles the statements inside a the function body
    compiler_block();
    object_function_t * function = compiler_end();
    compiler_emit_bytes(OP_CLOSURE, compiler_make_constant(OBJECT_VAL(function)));
    for (int32_t i = 0; i < function->upvalueCount; i++)
    {
        compiler_emit_byte(compiler.upvalues[i].isLocal ? 1 : 0);
        compiler_emit_byte(compiler.upvalues[i].index);
    }
}

/// @brief Compiles a function statement and defines the function in the current environment
static void compiler_function_declaration()
{
    uint8_t global = compiler_parse_variable("Expect function name.");
    compiler_mark_initialized();
    compiler_function(TYPE_FUNCTION);
    // Defines the function at the specified slot
    compiler_define_variable(global);
}

/// @brief Gets the parsing rule for a specific token type
/// @param type The type of the token
/// @return The parsing rule for the token type
static parse_rule_t *compiler_get_rule(tokentype_t type)
{
    return &rules[type];
}

/// @brief compiles a grouping expression '(expression)'
/// @param canAssign Unused for grouping expressions
static void compiler_grouping(bool canAssign)
{
    compiler_expression();
    compiler_consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

/// @brief Used to create a string object from an identifier token
/// @param name The name of the token
/// @return A byte code instruction that defines the constant
static uint8_t compiler_identifier_constant(token_t * name)
{
    return compiler_make_constant(OBJECT_VAL(object_copy_string(name->start, name->length, false)));
}

/// @brief Determines whether two identifiers are equal
/// @param a The first identifier token
/// @param b The second idientifier token
/// @return true if both identifiers are equal, false if not
static bool compiler_identifiers_equal(token_t * a, token_t * b)
{
    // If two identifiers have different lengths they must be different
    if (a->length != b->length)
        return false;
    return !memcmp(a->start, b->start, a->length);
}

/// @brief Compiles an if-statement
static void compiler_if_statement()
{
    compiler_consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    compiler_expression();
    compiler_consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
    // Offset to the instruction that corresponds to the body of the then block
    int32_t thenJump = compiler_emit_jump(OP_JUMP_IF_FALSE);
    compiler_statement();
    // Offset to the instruction that corresponds to the body of the else block
    int32_t elseJump = compiler_emit_jump(OP_JUMP);
    compiler_patch_jump(thenJump);
    compiler_emit_byte(OP_POP);
    if (compiler_match_token(TOKEN_ELSE))
        compiler_statement();
    // We patch that offset after the end of the else body or the end of the if body if no else is present
    compiler_patch_jump(elseJump);
}

/// @brief Initializes the compiler
/// @param compiler The compiler that is initialized
/// @param type The current type of function that is compiled (script for top level code / fuction for a function / init for a initializer / and method for a method)
static void compiler_init(compiler_t * compiler, function_type_t type)
{
    compiler->enclosing = current;
    compiler->function = NULL;
    compiler->type = type;
    compiler->localCount = compiler->scopeDepth = 0;
    compiler->function = object_new_function();
    current = compiler;
    if (type != TYPE_SCRIPT)
        current->function->name = object_copy_string(parser.previous.start, parser.previous.length, false);
    local_t * local = &current->locals[current->localCount++];
    local->depth = 0;
    local->isCaptured = false;
    // In a method we refer to locals with this
    if (type != TYPE_FUNCTION)
    {
        local->name.start = "this";
        local->name.length = 4;
    }
    else
    {
        local->name.start = "";
        local->name.length = 0;
    }
}

/// @brief Compiles a index of expression
/// @param canAssign Unsused for index of expressions
/// @param getOp Indicates whether the index of gets a value
/// @param setOp Indicates whether the index of sets a value
/// @param arg The index of the constant
static void compiler_index_of(bool canAssign, uint8_t getOp, uint8_t setOp, uint32_t arg)
{
    compiler_emit_bytes(getOp, (uint8_t)arg);
    compiler_expression();
    if(!compiler_match_token(TOKEN_RIGHT_BRACKET))
    {
        compiler_error("expected closing bracket ]");
        return;
    }
    // If an equal follows -> set index of expression
    if(compiler_match_token(TOKEN_EQUAL))
    {
        compiler_expression();
        compiler_emit_byte(OP_SET_INDEX_OF);
        compiler_emit_bytes(setOp, arg);
    }
    else
    {           
        compiler_emit_byte(OP_GET_INDEX_OF);
    }
}

/// @brief Compiles a boolean literal expression
/// @param canAssign Unused for boolean literal expressions
static void compiler_literal(bool canAssign)
{
    switch (parser.previous.type)
    {
    case TOKEN_FALSE:
        compiler_emit_byte(OP_FALSE);
        break;
    case TOKEN_NULL:
        compiler_emit_byte(OP_NULL);
        break;
    case TOKEN_TRUE:
        compiler_emit_byte(OP_TRUE);
        break;
    default:
        return; // Unreachable.
    }
}

/// @brief Marks a variable that already has been declared as initialized
/// @details This only applies to local variables
static void compiler_mark_initialized()
{
    if (!current->scopeDepth)
        return;
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

/// @brief Emits a constant bytecode instruction with the value that was passed as an argument up opon the function call
/// @param value The value of the constant
/// @return The index off the constant
static uint8_t compiler_make_constant(value_t value)
{
    int32_t constant = chunk_add_constant(compiler_current_chunk(), value);
    if (constant > (int32_t)UINT8_MAX)
    {
        // A chunk can only contain 255 constants
        compiler_error("Too many constants in one chunk.");
        return 0;
    }
    return (uint8_t)constant;
}

/// @brief Determines whether the next Token is from the specified TokenTypes and advances a position further, if that is the case
/// @param type The specified tokentype
/// @return true if the next token was from the given type
static bool compiler_match_token(tokentype_t type)
{
    if (!compiler_check(type))
        return false;
    compiler_advance();
    return true;
}

/// @brief Compiles a method declaration
static void compiler_method()
{
    compiler_consume(TOKEN_IDENTIFIER, "Expect method name.");
    uint8_t constant = compiler_identifier_constant(&parser.previous);
    function_type_t type = TYPE_METHOD;
    if (parser.previous.length == 4 && !memcmp(parser.previous.start, "init", 4))
        type = TYPE_INITIALIZER; // The initializer method, also called constructor in other languages, of a class.
    compiler_function(type);
    compiler_emit_bytes(OP_METHOD, constant);
}

/// @brief Handles getting and setting a variable (locals, globals and upvalues)
/// @param name The name of the variable
/// @param canAssign Boolean value that determines whether the value can be set 
static void compiler_named_variable(token_t name, bool canAssign)
{
    uint8_t getOp, setOp;
    int32_t arg = compiler_resolve_local(current, &name);
    if (arg != -1)
    {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    }
    else if ((arg = compiler_resolve_upvalue(current, &name)) != -1)
    {
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    }
    else
    {
        arg = compiler_identifier_constant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }
    if (canAssign && compiler_match_token(TOKEN_EQUAL))
    {
        compiler_expression();
        compiler_emit_bytes(setOp, (uint8_t)arg);
    }
    else if (canAssign && compiler_match_token(TOKEN_PLUS_EQUAL))
        compiler_nondirect_assignment(OP_ADD, getOp, setOp, arg);
    else if (canAssign && compiler_match_token(TOKEN_MINUS_EQUAL))
        compiler_nondirect_assignment(OP_SUBTRACT, getOp, setOp, arg);
    else if (canAssign && compiler_match_token(TOKEN_STAR_EQUAL))
        compiler_nondirect_assignment(OP_MULTIPLY, getOp, setOp, arg);
    else if (canAssign && compiler_match_token(TOKEN_SLASH_EQUAL))
        compiler_nondirect_assignment(OP_DIVIDE, getOp, setOp, arg);
    else if (canAssign && compiler_match_token(TOKEN_MODULO_EQUAL))
        compiler_nondirect_assignment(OP_MODULO, getOp, setOp, arg);
    else if (canAssign && compiler_match_token(TOKEN_STAR_STAR_EQUAL))
        compiler_nondirect_assignment(OP_EXPONENT, getOp, setOp, arg);
    else if(canAssign && compiler_match_token(TOKEN_LEFT_BRACKET))
        compiler_index_of(canAssign, getOp, setOp, arg);
    else
        compiler_emit_bytes(getOp, (uint8_t)arg);
}

/// @brief Compiles a nondirect assignment (e.g. +=/*=)
/// @param assignmentType The type of assignment
/// @param getOp The left operand (x += 5 -> x)
/// @param setOp The left operand (x += 5 -> x)
/// @param arg The right operand (x += 5 -> 5)
static void compiler_nondirect_assignment(uint8_t assignmentType, uint8_t getOp, uint8_t setOp, uint8_t arg)
{
    compiler_emit_bytes(getOp, arg);
    compiler_expression();
    compiler_emit_byte(assignmentType);
    compiler_emit_bytes(setOp, arg);
}

/// @brief compiles a number literal expression
/// @param canAssign Unused for number literal expressions
static void compiler_number(bool canAssign)
{
    double value = strtod(parser.previous.start, NULL);
    compiler_emit_constant(NUMBER_VAL(value));
}

/// @brief Compiles an or expression
/// @param canAssign Unused for or expressions
static void compiler_or(bool canAssign)
{
    // Jump to the segment that is executed if the condition is not satisfied
    int32_t elseJump = compiler_emit_jump(OP_JUMP_IF_FALSE);
    // Jump to the statements that are executed a condition is met in the or-expression
    int32_t endJump = compiler_emit_jump(OP_JUMP);
    compiler_patch_jump(elseJump);
    compiler_emit_byte(OP_POP);
    compiler_parse_precedence(PREC_OR);
    compiler_patch_jump(endJump);
}

/// @brief Parses the precedence of a statement to determine which function to call next, to compile the sourcecode
/// @param precedence The parsing precedence of the last compiled expression
static void compiler_parse_precedence(precedence_t precedence)
{
    compiler_advance();
    parse_function_t prefixRule = compiler_get_rule(parser.previous.type)->prefix;
    if (!prefixRule)
    {
        compiler_error("Expect expression.");
        return;
    }
    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);
    while (precedence <= compiler_get_rule(parser.current.type)->precedence)
    {
        compiler_advance();
        parse_function_t infixRule = compiler_get_rule(parser.previous.type)->infix;
        if(infixRule)
            infixRule(canAssign);
        else
            compiler_error("Invalid Token at the current position");
    }
    if (canAssign && compiler_match_token(TOKEN_EQUAL))
        compiler_error("Invalid assignment target.");
}

/// @brief Parses a variable statement
/// @param errorMessage The error message that is shown if the identifier is not valid
/// @return The slot of the local variable
static uint8_t compiler_parse_variable(char const * errorMessage)
{
    compiler_consume(TOKEN_IDENTIFIER, errorMessage);
    compiler_declare_variable();
    if (current->scopeDepth > 0)
        return 0;
    return compiler_identifier_constant(&parser.previous);
}

/// @brief Replaces the instruction at the given location with the calculated jump offset
/// @param offset The offset if the instruction
static void compiler_patch_jump(int32_t offset)
{
    // -2 to adjust for the bytecode for the jump offset itself.
    int32_t jump = compiler_current_chunk()->count - offset - 2;
    if (jump > (int32_t)UINT16_MAX)
        compiler_error("Too much code to jump over."); // More than 65,535 bytes of code
    // Jump offset (16-bit value) is split into two bytes
    compiler_current_chunk()->code[offset] = (jump >> 8) & 0xff;
    compiler_current_chunk()->code[offset + 1] = jump & 0xff;
}

/// @brief Resolves a local variable name
/// @param compiler The compiler where the local variable is resolved
/// @param name The name of tthe local variable
/// @return The index of the local variable
static int32_t compiler_resolve_local(compiler_t * compiler, token_t * name)
{
    for (int32_t i = compiler->localCount - 1; i >= 0; i--)
    {
        local_t *local = &compiler->locals[i];
        if (compiler_identifiers_equal(name, &local->name))
        {
            if (local->depth == -1)
                compiler_error("Can't read local variable in its own initializer.");
            return i;
        }
    }
    return -1;
}

/// @brief Looks for a local variable declared in any of the surrounding functions. 
/// @param compiler The compiler where the upvalue is attempted to be resolved
/// @param name The name of the local varoable the is resolved
/// @return If an upvalue is found it returns an upvalue index, if not -1 is returned.
static int32_t compiler_resolve_upvalue(compiler_t * compiler, token_t * name)
{
    if (!compiler->enclosing)
        return -1; // not found
    int32_t local = compiler_resolve_local(compiler->enclosing, name);
    if (local != -1)
    {
        compiler->enclosing->locals[local].isCaptured = true;
        return compiler_add_upvalue(compiler, (uint8_t)local, true);
    }
    // Resolution of a local variable failed in the current environent -> look in the enclosing environment
    int32_t upvalue = compiler_resolve_upvalue(compiler->enclosing, name);
    if (upvalue != -1)
        return compiler_add_upvalue(compiler, (uint8_t)upvalue, false);
    // upvalue_t couldn't be found
    return -1;
}

/// @brief Compiles a return statement
static void compiler_return_statement()
{
    if (current->type == TYPE_SCRIPT)
        compiler_error(" You can't use return from top-level code.");
    if (compiler_match_token(TOKEN_SEMICOLON))
        compiler_emit_return();
    else
    {
        if (current->type == TYPE_INITIALIZER)
            compiler_error("Can't return a value from an initializer. An initializer in cellox is not permitted to return a value.");
        compiler_expression();
        compiler_consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
        compiler_emit_byte(OP_RETURN);
    }
}

///@brief Compiles a statement
/// @details This can be either a for, an if, a return, a while, or a block statement.
/// If no statement was specified, the next expression is instead compiled
static void compiler_statement()
{
    if (compiler_match_token(TOKEN_FOR))
        compiler_for_statement();
    else if (compiler_match_token(TOKEN_IF))
        compiler_if_statement();
    else if (compiler_match_token(TOKEN_RETURN))
        compiler_return_statement();
    else if (compiler_match_token(TOKEN_WHILE))
        compiler_while_statement();
    else if (compiler_match_token(TOKEN_LEFT_BRACE))
    {
        compiler_begin_scope();
        compiler_block();
        compiler_end_scope();
    }
    else
        compiler_expression_statement();
}

/// @brief Compiles a string literal expression
/// @param canAssign Unused for string literals
/// @details The escape sequences in the string are resolved by this function.
/// If the string contains an unknowns escape sequence we show a compile error
static void compiler_string(bool canAssign)
{
    object_string_t * string = object_copy_string(parser.previous.start + 1, parser.previous.length - 2, true);
    if(!string)
    {
        compiler_error("Unknown escape sequence in string");
        return;
    }
    compiler_emit_constant(OBJECT_VAL(string));
}

/// @brief Compiles a super expression
/// @param canAssign Unused for super expression
static void compiler_super(bool canAssign)
{
    if (!currentClass)
        compiler_error("Can't use 'super' outside of a class.");
    else if (!currentClass->hasSuperclass)
        compiler_error("Can't use 'super' in a class with no superclass.");
    compiler_consume(TOKEN_DOT, "Expect '.' after 'super'.");
    compiler_consume(TOKEN_IDENTIFIER, "Expect superclass method name.");
    uint8_t name = compiler_identifier_constant(&parser.previous);
    compiler_named_variable(compiler_synthetic_token("this"), false);
    if (compiler_match_token(TOKEN_LEFT_PAREN))
    {
        uint8_t argCount = compiler_argument_list();
        compiler_named_variable(compiler_synthetic_token("super"), false);
        compiler_emit_bytes(OP_SUPER_INVOKE, name);
        compiler_emit_byte(argCount);
    }
    else
    {
        compiler_named_variable(compiler_synthetic_token("super"), false);
        compiler_emit_bytes(OP_GET_SUPER, name);
    }
}

/// @brief Synchronizes the compiler after an error has occured (jumps to the next statement that can be executed)
static void compiler_synchronize()
{
    parser.panicMode = false;
    while (parser.current.type != TOKEN_EOF)
    {        
        if (parser.previous.type == TOKEN_SEMICOLON)
            return;
        compiler_advance();
        switch (parser.current.type)
        {
        case TOKEN_CLASS:
        case TOKEN_FUN:
        case TOKEN_VAR:
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_RETURN:
            return;

        default:
        compiler_advance();
        }        
    }
}

/// @brief Creates a token out of a stream of characters
/// @param text The sequence of characters that is used to create a token
/// @return The created token
static token_t compiler_synthetic_token(char const * text)
{
    token_t token;
    token.start = text;
    token.length = (uint32_t)strlen(text);
    return token;
}

/// @brief Compiles a this expression
/// @param canAssign Unused for this expression
static void compiler_this(bool canAssign)
{
    if (!currentClass)
    {
        compiler_error("Can't use 'this' outside of a class.");
        return;
    }
    // This can not be reassigned
    compiler_variable(false);
}

/// @brief Compiles a unary expression
/// @param canAssign Unused for unary expression
static void compiler_unary(bool canAssign)
{
    tokentype_t operatorType = parser.previous.type;
    // Compile the operand.
    compiler_parse_precedence(PREC_UNARY);
    // Emit the operator instruction.
    switch (operatorType)
    {
    case TOKEN_BANG:
        compiler_emit_byte(OP_NOT);
        break;
    case TOKEN_MINUS:
        compiler_emit_byte(OP_NEGATE);
        break;
    default:
        return; // Unreachable.
    }
}

/// @brief Compiles a variable declaration
static void compiler_var_declaration()
{
    uint8_t global = compiler_parse_variable("Expect variable name.");
    if (compiler_match_token(TOKEN_EQUAL))
        compiler_expression(); // Variable was initialzed
    else
        compiler_emit_byte(OP_NULL); // Variable was not initialzed -> therefore is null
    compiler_consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    compiler_define_variable(global);
}

/// @brief Compiles a variable sets/gets global variable or sets/gets local variable
/// @param canAssign boolean value that determines whether a value can be aassigned to the variable
static void compiler_variable(bool canAssign)
{
    compiler_named_variable(parser.previous, canAssign);
}

/// @brief Compiles a while statement
/// @details The generated bytecode has the following structure
/// start:
/// if condition not met -> jump to the end
/// Instructions in the loop body
/// Jump to start
/// end:
static void compiler_while_statement()
{
    int32_t loopStart = compiler_current_chunk()->count;
    compiler_consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    // Compiles condition
    compiler_expression();
    compiler_consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
    int32_t exitJump = compiler_emit_jump(OP_JUMP_IF_FALSE);
    compiler_emit_byte(OP_POP);
    compiler_statement();
    compiler_emit_loop(loopStart);
    compiler_patch_jump(exitJump);
    compiler_emit_byte(OP_POP);
}
