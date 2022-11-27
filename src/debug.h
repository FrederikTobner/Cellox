#ifndef CELLOX_DEBUG_H_
#define CELLOX_DEBUG_H_

#include "chunk.h"

/// @brief  Dissasembles a chunk of bytecode instructions
/// @param chunk The chunk that is dissasembled
/// @param name The name of the chunk (based on the function)
/// @param arity The arity of the top level funtion of the chunk
void debug_disassemble_chunk(chunk_t * chunk, char const * name, uint32_t arity);

/// @brief Dissasembles a single instruction
/// @param chunk The chunk where a single instruction is dissasembled
/// @param offset The offset of the instruction
/// @return The offset of the next instruction
int32_t debug_disassemble_instruction(chunk_t *, int32_t);

#endif
