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
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    // Return from the current function
    OP_RETURN,

} Opcode;

// A dynamic array of idalized instructions for an abstract computer-> bytecode
typedef struct
{
    // Amount of bytecode instructions in the chunk
    int count;
    // Capacity of the chunk
    int capacity;
    // Operand Code
    uint8_t *code;
    // Line Array, stored seperatly to ensure there is no unnecesarry space taken up by
    int *lines;
    // Constants stored in the chunk
    ValueArray constants;
} Chunk;

// Initializes a chunk
void initChunk(Chunk *chunk);

// Write to a already existing chunk
void writeChunk(Chunk *chunk, uint8_t byte, int line);

// Adds a constant to the chunk
int addConstant(Chunk *chunk, Value value);

// Free's a chunk (Deallocates the memory used by the chunk)
void freeChunk(Chunk *chunk);

#endif