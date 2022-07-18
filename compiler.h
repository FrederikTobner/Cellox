#ifndef clox_compiler_h
#define clox_compiler_h

#include "object.h"

// Compiles a chunk of bytecode
ObjFunction *compile(const char *source);
//Marks the compiler roots
void markCompilerRoots();

#endif