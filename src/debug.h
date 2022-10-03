#ifndef cellox_debug_h
#define cellox_debug_h

#include "chunk.h"

// Dissasembles a chunk of bytecode instructions
void debug_disassemble_chunk(Chunk * chunk, char const * name);

// Dissasembles a single instruction
int32_t debug_disassemble_instruction(Chunk * chunk, int32_t offset);

#endif