#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

// opcodes of the bytecode instruction set
typedef enum
{
    // Defines a Constant
    OP_CONSTANT,
    // Adds the two constants on the top of the stack
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,
    OP_NULL,
    OP_TRUE,
    OP_FALSE,
    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    // Negate the value on top of the stack
    OP_NEGATE,
    // Prints the value on top of the stack
    OP_PRINT,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_INVOKE,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE,
    // Return from the current function
    OP_RETURN,
    OP_CLASS,
    OP_METHOD,
    OP_GET_SUPER,
    OP_SUPER_INVOKE,
    OP_INHERIT,
} Opcode;

/* Type definition of a dynamic array structure of bytecode instructions
bytecode instructions are idalized instructions for an abstract/virtual computer*/
typedef struct
{
    // Amount of bytecode instructions in the chunk
    int32_t count;
    // Capacity of the chunk
    int32_t capacity;
    // Operand Codes
    uint8_t *code;
    // STores line information to the corresponding lox program
    int32_t *lines;
    // Constants stored in the chunk
    ValueArray constants;
} Chunk;

// Adds a constant to the chunk
int32_t addConstant(Chunk *chunk, Value value);

// Free's a chunk (Deallocates the memory used by the chunk)
void freeChunk(Chunk *chunk);

// Initializes a chunk
void initChunk(Chunk *chunk);

// Write to a already existing chunk
void writeChunk(Chunk *chunk, uint8_t byte, int32_t line);

#endif