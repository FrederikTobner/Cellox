#ifndef clox_compiler_h
#define clox_compiler_h

#include "vm.h"
#include "object.h"

// Compiles a chunk of bytecode
ObjFunction *compile(const char *source);

#endif