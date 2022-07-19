#ifndef clox_compiler_h
#define clox_compiler_h

#include "object.h"

// Compiles a chunk of bytecode
ObjFunction *compile(const char *source);

/* Marks the compiler roots.
 * These are all the the objects that can be directly accessed by the vm
 * and not through a reference in some other object
 */
void markCompilerRoots();

#endif