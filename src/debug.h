#ifndef CELLOX_DEBUG_H_
#define CELLOX_DEBUG_H_

#include "chunk.h"

// Dissasembles a chunk of bytecode instructions
void debug_disassemble_chunk(chunk_t * chunk, char const * name);

// Dissasembles a single instruction
int32_t debug_disassemble_instruction(chunk_t * chunk, int32_t offset);

#endif