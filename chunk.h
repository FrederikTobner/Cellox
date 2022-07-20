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
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    // Negate a Value
    OP_NEGATE,
    OP_PRINT,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE,
    // Return from the current function
    OP_RETURN,
} Opcode;

/* Type definition of a dynamic array structure of bytecode instructions
bytecode instructions are idalized instructions for an abstract/virtual computer*/
typedef struct
{
    // Amount of bytecode instructions in the chunk
    int32_t count;
    // Capacity of the chunk
    int32_t capacity;
    // Operand Code
    uint8_t *code;
    // Line Array, stored seperatly to ensure there is no unnecesarry space taken up by
    int32_t *lines;
    // Constants stored in the chunk
    ValueArray constants;
} Chunk;

// Initializes a chunk
void initChunk(Chunk *chunk);

// Write to a already existing chunk
void writeChunk(Chunk *chunk, uint8_t byte, int32_t line);

// Adds a constant to the chunk
int32_t addConstant(Chunk *chunk, Value value);

// Free's a chunk (Deallocates the memory used by the chunk)
void freeChunk(Chunk *chunk);

#endif