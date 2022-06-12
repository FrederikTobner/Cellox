#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"

// opcodes of the language
typedef enum {
    //Return from the current function
    OP_RETURN,
    
}Opcode;

// A dynamic array of intstructions -> bytecode
typedef struct {
    int count;
    int capacity;
    uint8_t* code;
}Chunk;

// Initializes a chunk
void initChunk(Chunk* chunk);

// Write to a already existing chunk
void writeChunk(Chunk* chunk, uint8_t byte);

// Free's a chunk (Deallocates the memory used by the chunk)
void freeChunk(Chunk* chunk);

#endif