#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "memory.h"
#include "lexer.h"
#include "vm.h"

// The debug header file only needs to be included if the DEBUG_CODE is defined
#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

// Type definition of the parser structure
typedef struct
{
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

// Precedences of the Tokens
typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT, // = += -= *= /= %= **=
    PREC_OR,         // or ||
    PREC_AND,        // and  &&
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM,       // + -
    PREC_FACTOR,     // * / % **
    PREC_UNARY,      // ! - ++x, --x
    PREC_CALL,       // . ()
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool canAssign);

// Type definition of a parsing rule structure
typedef struct
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

// Type definition of a local variable structure
typedef struct
{
    // Name of the local variable
    Token name;
    // Scope depth where the local variable was declared
    int32_t depth;
    // Boolean value that determines whether the local variable is captured by a closure
    bool isCaptured;
} Local;

// Type definition of an upvalue structure
typedef struct
{
    uint8_t index;
    bool isLocal;
} Upvalue;

// Types of a function - either a script or a function
typedef enum
{
    TYPE_FUNCTION,
    TYPE_INITIALIZER,
    TYPE_METHOD,
    TYPE_SCRIPT
} FunctionType;

// Type definition of the compiler structure
typedef struct Compiler
{
    struct Compiler *enclosing;
    ObjectFunction *function;
    FunctionType type;
    Local locals[UINT8_COUNT];
    int32_t localCount;
    Upvalue upvalues[UINT8_COUNT];
    int32_t scopeDepth;
} Compiler;

typedef struct ClassCompiler
{
    struct ClassCompiler *enclosing;
    bool hasSuperclass;
} ClassCompiler;

// Global parser variable
Parser parser;

// Global compiler variable
Compiler *current = NULL;

// Global classCompiler variable
ClassCompiler *currentClass = NULL;

static void addLocal(Token);
static int32_t addUpvalue(Compiler *, uint8_t, bool);
static void advance();
static void and_(bool);
static uint8_t argumentList();
static void beginScope();
static void binary(bool);
static void block();
static void call(bool);
static bool check(TokenType);
static void classDeclaration();
static void consume(TokenType, const char *);
static Chunk *currentChunk();
static void declaration();
static void declareVariable();
static void defineVariable(uint8_t);
static void dot(bool);
static void emitByte(uint8_t);
static void emitBytes(uint8_t, uint8_t);
static void emitConstant(Value);
static int32_t emitJump(uint8_t);
static void emitLoop(int32_t);
static void emitReturn();
static ObjectFunction *endCompiler();
static void endScope();
static void error(const char *);
static void errorAt(Token *, const char *);
static void errorAtCurrent(const char *);
static void expression();
static void expressionStatement();
static void forStatement();
static void function(FunctionType);
static void funDeclaration();
static ParseRule *getRule(TokenType);
static void grouping(bool);
static uint8_t identifierConstant(Token *);
static bool identifiersEqual(Token *, Token *);
static void ifStatement();
static void initCompiler(Compiler *, FunctionType);
static void literal(bool);
static void markInitialized();
static uint8_t makeConstant(Value);
static bool match(TokenType);
static void method();
static void namedVariable(Token, bool);
static void nonDirectAssignment(uint8_t, uint8_t, uint8_t, int32_t);
static void number(bool);
static void or_(bool);
static void parsePrecedence(Precedence);
static uint8_t parseVariable(const char *);
static void patchJump(int32_t);
static void printStatement();
static int32_t resolveLocal(Compiler *, Token *);
static int32_t resolveUpvalue(Compiler *, Token *);
static void returnStatement();
static void statement();
static void string(bool);
static void super_(bool);
static void synchronize();
static Token syntheticToken(const char *);
static void this_(bool);
static void unary(bool);
static void varDeclaration();
static void variable(bool);
static void whileStatement();

ObjectFunction *compile(const char *source)
{
    initLexer(source);
    Compiler compiler;
    initCompiler(&compiler, TYPE_SCRIPT);
    parser.hadError = false;
    parser.panicMode = false;
    advance();
    // We keep compiling until we hit the end of the source file
    while (!match(TOKEN_EOF))
        declaration();
    ObjectFunction *function = endCompiler();
    return parser.hadError ? NULL : function;
}

void markCompilerRoots()
{
    Compiler *compiler = current;
    while (compiler != NULL)
    {
        markObject((Object *)compiler->function);
        compiler = compiler->enclosing;
    }
}

// ParseRules for the language
ParseRule rules[] = {
    [TOKEN_AND] = {NULL, and_, PREC_AND},
    [TOKEN_BANG] = {unary, NULL, PREC_UNARY},
    [TOKEN_BANG_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_CLASS] = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT] = {NULL, dot, PREC_CALL},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE] = {literal, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_FUN] = {NULL, NULL, PREC_NONE},
    [TOKEN_GREATER] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER] = {variable, NULL, PREC_NONE},
    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_PAREN] = {grouping, call, PREC_CALL},
    [TOKEN_LESS] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_MODULO] = {NULL, binary, PREC_FACTOR},
    [TOKEN_MINUS] = {unary, binary, PREC_TERM},
    [TOKEN_NULL] = {literal, NULL, PREC_NONE},
    [TOKEN_NUMBER] = {number, NULL, PREC_NONE},
    [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
    [TOKEN_OR] = {NULL, or_, PREC_OR},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR_STAR] = {NULL, binary, PREC_FACTOR},
    [TOKEN_STRING] = {string, NULL, PREC_NONE},
    [TOKEN_SUPER] = {super_, NULL, PREC_NONE},
    [TOKEN_THIS] = {this_, NULL, PREC_NONE},
    [TOKEN_TRUE] = {literal, NULL, PREC_NONE},
    [TOKEN_VAR] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},

};

// Adds a new local variable to the stack
static void addLocal(Token name)
{
    if (current->localCount == UINT8_COUNT)
    {
        // We can only have 256 objects on the stack 😔
        error("Too many local variables in function.");
        return;
    }
    Local *local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
    local->isCaptured = false;
}

// Adds an upValue to the compiler
static int32_t addUpvalue(Compiler *compiler, uint8_t index, bool isLocal)
{
    int32_t upvalueCount = compiler->function->upvalueCount;

    for (int32_t i = 0; i < upvalueCount; i++)
    {
        Upvalue *upvalue = &compiler->upvalues[i];
        if (upvalue->index == index && upvalue->isLocal == isLocal)
            return i;
    }
    if (upvalueCount == UINT8_COUNT)
    {
        error("Too many closure variables in function.");
        return 0;
    }

    compiler->upvalues[upvalueCount].isLocal = isLocal;
    compiler->upvalues[upvalueCount].index = index;
    return compiler->function->upvalueCount++;
}

// Advances a poosition further in the linea sequence of tokens
static void advance()
{
    parser.previous = parser.current;

    for (;;)
    {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR)
            break;

        errorAtCurrent(parser.current.start);
    }
}

// Handles an and-expression
static void and_(bool canAssign)
{
    int32_t endJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    parsePrecedence(PREC_AND);
    patchJump(endJump);
}

// Compiles an argumentList of a call expression to bytecode instructions
static uint8_t argumentList()
{
    uint8_t argCount = 0;
    if (!check(TOKEN_RIGHT_PAREN))
    {
        do
        {
            expression();
            if (argCount == 255)
                error("Can't have more than 255 arguments.");
            argCount++;
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
    return argCount;
}

// Handles the beginning of a new Scope
static void beginScope()
{
    current->scopeDepth++;
}

// Compiles a binary expression
static void binary(bool canAssign)
{
    TokenType operatorType = parser.previous.type;
    ParseRule *rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operatorType)
    {
    case TOKEN_BANG_EQUAL:
        emitBytes(OP_EQUAL, OP_NOT);
        break;
    case TOKEN_EQUAL_EQUAL:
        emitByte(OP_EQUAL);
        break;
    case TOKEN_GREATER:
        emitByte(OP_GREATER);
        break;
    case TOKEN_GREATER_EQUAL:
        emitBytes(OP_LESS, OP_NOT);
        break;
    case TOKEN_LESS:
        emitByte(OP_LESS);
        break;
    case TOKEN_LESS_EQUAL:
        emitBytes(OP_GREATER, OP_NOT);
        break;
    case TOKEN_PLUS:
        emitByte(OP_ADD);
        break;
    case TOKEN_MINUS:
        emitByte(OP_SUBTRACT);
        break;
    case TOKEN_STAR:
        emitByte(OP_MULTIPLY);
        break;
    case TOKEN_SLASH:
        emitByte(OP_DIVIDE);
        break;
    case TOKEN_MODULO:
        emitByte(OP_MODULO);
        break;
    case TOKEN_STAR_STAR:
        emitByte(OP_EXPONENT);
        break;
    default:
        return; // Unreachable.
    }
}

// Compiles a block statement to bytecode instructions
static void block()
{
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
        declaration();
    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

// Compiles a call expression
static void call(bool canAssign)
{
    uint8_t argCount = argumentList();
    emitBytes(OP_CALL, argCount);
}

// Checks if the next Token is of a given type
static bool check(TokenType type)
{
    return parser.current.type == type;
}

// Parses a new class declaration
static void classDeclaration()
{
    consume(TOKEN_IDENTIFIER, "Expect class name.");
    Token className = parser.previous;
    uint8_t nameConstant = identifierConstant(&parser.previous);
    declareVariable();
    emitBytes(OP_CLASS, nameConstant);
    defineVariable(nameConstant);
    ClassCompiler classCompiler;
    classCompiler.hasSuperclass = false;
    classCompiler.enclosing = currentClass;
    currentClass = &classCompiler;
    if (match(TOKEN_DOUBLEDOT))
    {
        consume(TOKEN_IDENTIFIER, "Expect superclass name.");
        variable(false);
        if (identifiersEqual(&className, &parser.previous))
            error("A class can't inherit from itself.");
        beginScope();
        addLocal(syntheticToken("super"));
        defineVariable(0);
        namedVariable(className, false);
        emitByte(OP_INHERIT);
        classCompiler.hasSuperclass = true;
    }
    namedVariable(className, false);
    consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
        method();
    consume(TOKEN_RIGHT_BRACE, "Expect '}' after class body.");
    emitByte(OP_POP);
    if (classCompiler.hasSuperclass)
        endScope();
    currentClass = currentClass->enclosing;
}

// Consumes a Token
static void consume(TokenType type, const char *message)
{
    if (parser.current.type == type)
    {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static Chunk *currentChunk()
{
    return &current->function->chunk;
}

// Compiles a declaration stament or a normal statement
static void declaration()
{
    if (match(TOKEN_CLASS))
        classDeclaration();
    else if (match(TOKEN_FUN))
        funDeclaration();
    else if (match(TOKEN_VAR))
        varDeclaration();
    else
        statement();
    if (parser.panicMode)
        synchronize();
}

// Declares a new variable
static void declareVariable()
{
    // Global scope
    if (current->scopeDepth == 0)
        return;

    Token *name = &parser.previous;
    for (int32_t i = current->localCount - 1; i >= 0; i--)
    {
        Local *local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scopeDepth)
            break;

        if (identifiersEqual(name, &local->name))
            error("Already a variable with this name in this scope.");
    }
    addLocal(*name);
}

// Defines a new Variable
static void defineVariable(uint8_t global)
{
    // Local Variable
    if (current->scopeDepth > 0)
    {
        markInitialized();
        return;
    }
    emitBytes(OP_DEFINE_GLOBAL, global);
}

// Compiles a dot statement (get or set)
static void dot(bool canAssign)
{
    consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
    uint8_t name = identifierConstant(&parser.previous);

    if (canAssign && match(TOKEN_EQUAL))
    {
        expression();
        emitBytes(OP_SET_PROPERTY, name);
    }
    else if (match(TOKEN_LEFT_PAREN))
    {
        uint8_t argCount = argumentList();
        emitBytes(OP_INVOKE, name);
        emitByte(argCount);
    }
    else
    {
        emitBytes(OP_GET_PROPERTY, name);
    }
}

// Emits a single byte as bytecode instructionn
static void emitByte(uint8_t byte)
{
    writeChunk(currentChunk(), byte, parser.previous.line);
}

// Emits two bytes as bytecode instructions
static void emitBytes(uint8_t byte1, uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

// Creates a constant bytecode instruction
static void emitConstant(Value value)
{
    emitBytes(OP_CONSTANT, makeConstant(value));
}

/* Emits a bytecode instruction of the type jump (jump or jump-if-false)
 * and writes a placeholder to the jump offset
 * Additionally returns the offset (start address) of the then or else branch
 */
static int32_t emitJump(uint8_t instruction)
{
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->count - 2;
}

// Emits the bytecode instructions for creating a loop
static void emitLoop(int32_t loopStart)
{
    emitByte(OP_LOOP);

    int32_t offset = currentChunk()->count - loopStart + 2;
    if (offset > (int32_t)UINT16_MAX)
        error("Loop body too large.");
    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

// Emits a return bytecode instruction
static void emitReturn()
{
    if (current->type == TYPE_INITIALIZER)
        emitBytes(OP_GET_LOCAL, 0);
    else
        emitByte(OP_NULL);
    emitByte(OP_RETURN);
}

// yields a newly created function object after the compilation process finished
static ObjectFunction *endCompiler()
{
    emitReturn();
    ObjectFunction *function = current->function;
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError)
        disassembleChunk(currentChunk(), function->name != NULL ? function->name->chars : "<script>");
#endif
    current = current->enclosing;
    return function;
}

// Handles the closing of a scope
static void endScope()
{
    current->scopeDepth--;

    /*We walk backward through the local array looking for any variables
     declared at the scope depth we just left, when we pop a scope*/
    while (current->localCount > 0 &&
           current->locals[current->localCount - 1].depth > current->scopeDepth)
    {
        if (current->locals[current->localCount - 1].isCaptured)
            emitByte(OP_CLOSE_UPVALUE);
        else
            emitByte(OP_POP);
        current->localCount--;
    }
}

// Reports an error at the previous position
static void error(const char *message)
{
    errorAt(&parser.previous, message);
}

// Reports an error that was triggered by a specifiec token
static void errorAt(Token *token, const char *message)
{
    if (parser.panicMode)
        return;
    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF)
        fprintf(stderr, " at end");
    else
        fprintf(stderr, " at '%.*s'", token->length, token->start);

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

// Reports an error at the current position
static void errorAtCurrent(const char *message)
{
    errorAt(&parser.current, message);
}

// Compiles a expression
static void expression()
{
    parsePrecedence(PREC_ASSIGNMENT);
}

// Compiles an expression-statement
static void expressionStatement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

// Compiles a for-statement
static void forStatement()
{
    beginScope();
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

    // Initializer clause
    if (match(TOKEN_SEMICOLON))
    {
        // No initializer.
    }
    else if (match(TOKEN_VAR))
    {
        varDeclaration();
    }
    else
    {
        expressionStatement();
    }

    int32_t loopStart = currentChunk()->count;
    int32_t exitJump = -1;

    // Conditional clause
    if (!match(TOKEN_SEMICOLON))
    {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        // Jump out of the loop if the condition is false.
        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP); // Condition.
    }

    // Increment clause
    if (!match(TOKEN_RIGHT_PAREN))
    {
        int32_t bodyJump = emitJump(OP_JUMP);
        int32_t incrementStart = currentChunk()->count;
        expression();
        emitByte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement();
    emitLoop(loopStart);

    if (exitJump != -1)
    {
        patchJump(exitJump);
        emitByte(OP_POP); // Condition.
    }

    endScope();
}

// Compiles a function declaration statement to bytecode instructions
static void function(FunctionType type)
{
    Compiler compiler;
    initCompiler(&compiler, type);
    beginScope();

    consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");
    if (!check(TOKEN_RIGHT_PAREN))
    {
        do
        {
            current->function->arity++;
            if (current->function->arity > 255)
                errorAtCurrent("Can't have more than 255 parameters.");
            uint8_t constant = parseVariable("Expect parameter name.");
            defineVariable(constant);
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    block();

    ObjectFunction *function = endCompiler();
    emitBytes(OP_CLOSURE, makeConstant(OBJECT_VAL(function)));
    for (int32_t i = 0; i < function->upvalueCount; i++)
    {
        emitByte(compiler.upvalues[i].isLocal ? 1 : 0);
        emitByte(compiler.upvalues[i].index);
    }
}

// Compiles a function statement and defines the function in the current environment
static void funDeclaration()
{
    uint8_t global = parseVariable("Expect function name.");
    markInitialized();
    function(TYPE_FUNCTION);
    defineVariable(global);
}

// Gets the parsing rule for a specific token type
static ParseRule *getRule(TokenType type)
{
    return &rules[type];
}

// compiles a grouping expression '(expression)'
static void grouping(bool canAssign)
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

// Used to create a string object from an identifier token
static uint8_t identifierConstant(Token *name)
{
    return makeConstant(OBJECT_VAL(copyString(name->start, name->length, false)));
}

// Determines whether two identifiers are equal
static bool identifiersEqual(Token *a, Token *b)
{
    if (a->length != b->length)
        return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

// Compiles an if-statement
static void ifStatement()
{
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int32_t thenJump = emitJump(OP_JUMP_IF_FALSE);
    statement();
    int32_t elseJump = emitJump(OP_JUMP);
    patchJump(thenJump);
    emitByte(OP_POP);
    if (match(TOKEN_ELSE))
        statement();

    // We patch that offset after the end of the else body
    patchJump(elseJump);
}

// Initializes the compiler
static void initCompiler(Compiler *compiler, FunctionType type)
{
    compiler->enclosing = current;
    compiler->function = NULL;
    compiler->type = type;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = newFunction();
    current = compiler;
    if (type != TYPE_SCRIPT)
        current->function->name = copyString(parser.previous.start, parser.previous.length, false);
    Local *local = &current->locals[current->localCount++];
    local->depth = 0;
    local->isCaptured = false;
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

// Compiles a boolean literal expression
static void literal(bool canAssign)
{
    switch (parser.previous.type)
    {
    case TOKEN_FALSE:
        emitByte(OP_FALSE);
        break;
    case TOKEN_NULL:
        emitByte(OP_NULL);
        break;
    case TOKEN_TRUE:
        emitByte(OP_TRUE);
        break;
    default:
        return; // Unreachable.
    }
}

// Marks a variable that already has been declared as initialized
static void markInitialized()
{
    if (current->scopeDepth == 0)
        return;
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

// Emits a constant bytecode instruction with the value that was passed as an argument up opon the function call
static uint8_t makeConstant(Value value)
{
    int32_t constant = addConstant(currentChunk(), value);
    if (constant > (int32_t)UINT8_MAX)
    {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

// Determines weather the next Token is from the specified TokenTypes and advances a position further, if that is the case
static bool match(TokenType type)
{
    if (!check(type))
        return false;
    advance();
    return true;
}

// Compiles a method declaration
static void method()
{
    consume(TOKEN_IDENTIFIER, "Expect method name.");
    uint8_t constant = identifierConstant(&parser.previous);
    FunctionType type = TYPE_METHOD;
    if (parser.previous.length == 4 && memcmp(parser.previous.start, "init", 4) == 0)
        type = TYPE_INITIALIZER;

    function(type);
    emitBytes(OP_METHOD, constant);
}

// Handles getting and setting a variable (locals, globals and upvalues)
static void namedVariable(Token name, bool canAssign)
{
    uint8_t getOp, setOp;
    int32_t arg = resolveLocal(current, &name);
    if (arg != -1)
    {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    }
    else if ((arg = resolveUpvalue(current, &name)) != -1)
    {
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    }
    else
    {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }
    if (canAssign && match(TOKEN_EQUAL))
    {
        expression();
        emitBytes(setOp, (uint8_t)arg);
    }
    else if (canAssign && match(TOKEN_PLUS_EQUAL))
        nonDirectAssignment(OP_ADD, getOp, setOp, arg);
    else if (canAssign && match(TOKEN_MINUS_EQUAL))
        nonDirectAssignment(OP_SUBTRACT, getOp, setOp, arg);
    else if (canAssign && match(TOKEN_STAR_EQUAL))
        nonDirectAssignment(OP_MULTIPLY, getOp, setOp, arg);
    else if (canAssign && match(TOKEN_SLASH_EQUAL))
        nonDirectAssignment(OP_DIVIDE, getOp, setOp, arg);
    else if (canAssign && match(TOKEN_MODULO_EQUAL))
        nonDirectAssignment(OP_MODULO, getOp, setOp, arg);
    else if (canAssign && match(TOKEN_STAR_STAR_EQUAL))
        nonDirectAssignment(OP_EXPONENT, getOp, setOp, arg);
    else
        emitBytes(getOp, (uint8_t)arg);
}

static void nonDirectAssignment(uint8_t assignmentType, uint8_t getOp, uint8_t setOp, int32_t arg)
{
    emitBytes(getOp, (uint8_t)arg);
    expression();
    emitByte(assignmentType);
    emitBytes(setOp, (uint8_t)arg);
}

// compiles a number literal expression
static void number(bool canAssign)
{
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

// Handles an or-expression
static void or_(bool canAssign)
{
    int32_t elseJump = emitJump(OP_JUMP_IF_FALSE);
    int32_t endJump = emitJump(OP_JUMP);

    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(PREC_OR);
    patchJump(endJump);
}

// Parses the precedence of a statement to determine whether a valid assignment target is specified
static void parsePrecedence(Precedence precedence)
{
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL)
    {
        error("Expect expression.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);
    while (precedence <= getRule(parser.current.type)->precedence)
    {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }
    if (canAssign && match(TOKEN_EQUAL))
        error("Invalid assignment target.");
}

// Parses a variable statement
static uint8_t parseVariable(const char *errorMessage)
{
    consume(TOKEN_IDENTIFIER, errorMessage);
    declareVariable();
    if (current->scopeDepth > 0)
        return 0;
    return identifierConstant(&parser.previous);
}

// Replaces the operand at the given location with the calculated jump offset
static void patchJump(int32_t offset)
{
    // -2 to adjust for the bytecode for the jump offset itself.
    int32_t jump = currentChunk()->count - offset - 2;
    if (jump > (int32_t)UINT16_MAX)
        error("Too much code to jump over."); // More than 65,535 bytes of code
    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

// Compiles a print statement
static void printStatement()
{
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

// Resolves a local variable name
static int32_t resolveLocal(Compiler *compiler, Token *name)
{
    for (int32_t i = compiler->localCount - 1; i >= 0; i--)
    {
        Local *local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name))
        {
            if (local->depth == -1)
                error("Can't read local variable in its own initializer.");
            return i;
        }
    }
    return -1;
}

/* Looks for a local variable declared in any of the surrounding functions.
If an upvalue is found it returns an upvalue index, if not -1 is returned.*/
static int32_t resolveUpvalue(Compiler *compiler, Token *name)
{
    if (compiler->enclosing == NULL)
        return -1; // not found

    int32_t local = resolveLocal(compiler->enclosing, name);
    if (local != -1)
    {
        compiler->enclosing->locals[local].isCaptured = true;
        return addUpvalue(compiler, (uint8_t)local, true);
    }
    // Resolution of a local variable failed in the current environent -> look in the enclosing environment
    int32_t upvalue = resolveUpvalue(compiler->enclosing, name);
    if (upvalue != -1)
        return addUpvalue(compiler, (uint8_t)upvalue, false);
    // Upvalue couldn't be found
    return -1;
}

// Compiles a return statement
static void returnStatement()
{
    if (current->type == TYPE_SCRIPT)
        error("Can't return from top-level code.");
    if (match(TOKEN_SEMICOLON))
        emitReturn();
    else
    {
        if (current->type == TYPE_INITIALIZER)
            error("Can't return a value from an initializer.");
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
        emitByte(OP_RETURN);
    }
}

// Compiles a statement
static void statement()
{
    if (match(TOKEN_PRINT))
        printStatement();
    else if (match(TOKEN_FOR))
        forStatement();
    else if (match(TOKEN_IF))
        ifStatement();
    else if (match(TOKEN_RETURN))
        returnStatement();
    else if (match(TOKEN_WHILE))
        whileStatement();
    else if (match(TOKEN_LEFT_BRACE))
    {
        beginScope();
        block();
        endScope();
    }
    else
        expressionStatement();
}

// compiles a string literal expression
static void string(bool canAssign)
{
    emitConstant(OBJECT_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2, true)));
}

// Compiles a super expression
static void super_(bool canAssign)
{
    if (currentClass == NULL)
        error("Can't use 'super' outside of a class.");
    else if (!currentClass->hasSuperclass)
        error("Can't use 'super' in a class with no superclass.");
    consume(TOKEN_DOT, "Expect '.' after 'super'.");
    consume(TOKEN_IDENTIFIER, "Expect superclass method name.");
    uint8_t name = identifierConstant(&parser.previous);

    namedVariable(syntheticToken("this"), false);
    if (match(TOKEN_LEFT_PAREN))
    {
        uint8_t argCount = argumentList();
        namedVariable(syntheticToken("super"), false);
        emitBytes(OP_SUPER_INVOKE, name);
        emitByte(argCount);
    }
    else
    {
        namedVariable(syntheticToken("super"), false);
        emitBytes(OP_GET_SUPER, name);
    }
}

// Synchronizes the compiler after an error has occured (jumps to the next statement)
static void synchronize()
{
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF)
    {
        if (parser.previous.type == TOKEN_SEMICOLON)
            return;
        switch (parser.current.type)
        {
        case TOKEN_CLASS:
        case TOKEN_FUN:
        case TOKEN_VAR:
        case TOKEN_FOR:
        case TOKEN_IF:
        case TOKEN_WHILE:
        case TOKEN_PRINT:
        case TOKEN_RETURN:
            return;

        default:;
        }

        advance();
    }
}

static Token syntheticToken(const char *text)
{
    Token token;
    token.start = text;
    token.length = (int)strlen(text);
    return token;
}

// Compiles a this expression
static void this_(bool canAssign)
{
    if (currentClass == NULL)
    {
        error("Can't use 'this' outside of a class.");
        return;
    }

    variable(false);
}

// Compiles a unary expression
static void unary(bool canAssign)
{
    TokenType operatorType = parser.previous.type;
    // Compile the operand.
    parsePrecedence(PREC_UNARY);
    // Emit the operator instruction.
    switch (operatorType)
    {
    case TOKEN_BANG:
        emitByte(OP_NOT);
        break;
    case TOKEN_MINUS:
        emitByte(OP_NEGATE);
        break;
    default:
        return; // Unreachable.
    }
}

// Compiles a variable declaration
static void varDeclaration()
{
    uint8_t global = parseVariable("Expect variable name.");
    if (match(TOKEN_EQUAL))
        expression(); // Variable was initialzed
    else
        emitByte(OP_NULL); // Variable was not initialzed
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    defineVariable(global);
}

/*Compiles a variable
* Get Global variable or
* Set Global variable or
* Get Local variable or
Set Local variable*/
static void variable(bool canAssign)
{
    namedVariable(parser.previous, canAssign);
}

// Compiles a while statement
static void whileStatement()
{
    int32_t loopStart = currentChunk()->count;
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
    int32_t exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    emitLoop(loopStart);
    patchJump(exitJump);
    emitByte(OP_POP);
}