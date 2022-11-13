#ifndef CELLOX_COMPILER_H_
#define CELLOX_COMPILER_H_

#include "object.h"

/// @brief Compiles a cellox program
/// @param code The cellox program that is compiled
/// @return A obect_function_t that that stores all the instructions that of the cellox program
object_function_t * compiler_compile(char const * code);

/// @brief Marks the compiler roots.
/// @details These are all the the objects that can be directly accessed by the virtualMachine and not through a reference in some other object..
void compiler_mark_roots();

#endif
