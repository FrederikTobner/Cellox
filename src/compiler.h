#ifndef cellox_compiler_h
#define cellox_compiler_h

#include "object.h"

// Compiles a chunk of bytecode
ObjectFunction * compiler_compile(char const * source);

/* Marks the compiler roots.
 * These are all the the objects that can be directly accessed by the virtualMachine
 * and not through a reference in some other object
 */
void compiler_mark_roots();

#endif