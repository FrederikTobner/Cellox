#ifndef clox_compiler_h
#define clox_compiler_h

#include "vm.h"

// Compiles a chunk of bytecode
bool compile(const char *source, Chunk *chunk);

#endif