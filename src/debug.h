#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"

// Dissasembles a chunk of bytecode instructions
void disassembleChunk(Chunk *chunk, const char *name);

// Dissasembles a single instruction
int32_t disassembleInstruction(Chunk *chunk, int32_t offset);

#endif