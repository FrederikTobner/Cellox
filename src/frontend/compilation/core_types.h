/****************************************************************************
 * Copyright (C) 2022 by Frederik Tobner                                    *
 *                                                                          *
 * This file is part of Cellox.                                             *
 *                                                                          *
 * Permission to use, copy, modify, and distribute this software and its    *
 * documentation under the terms of the GNU General Public License is       *
 * hereby granted.                                                          *
 * No representations are made about the suitability of this software for   *
 * any purpose.                                                             *
 * It is provided "as is" without express or implied warranty.              *
 * See the <"https://www.gnu.org/licenses/gpl-3.0.html">GNU General Public  *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file core_types.h
 * @brief Core compiler type declarations shared across compiler modules.
 */

#ifndef CELLOX_CORE_TYPES_H_
#define CELLOX_CORE_TYPES_H_

#include "compiler.h"

#include <stdint.h>

/// @brief A local variable structure
typedef struct {
    /// Name of the local variable
    token_t name;
    /// Scope depth where the local variable was declared
    int32_t depth;
    /// Boolean value that determines whether the local variable is captured by a closure
    bool isCaptured;
} local_t;

/// @brief An upvalue structure
typedef struct {
    /// Index of the upvalue
    uint8_t index;
    /// Flag that indicates whether the value is a local value
    bool isLocal;
} upvalue_t;

/// @brief A cellox function
typedef enum {
    /// Marks a normal function
    TYPE_FUNCTION,
    /// @brief Marks an initializer function
    /// @details In other programming languages this often called a contstructor
    TYPE_INITIALIZER,
    /// A method that is bound to a class
    TYPE_METHOD,
    /// A cellox script
    TYPE_SCRIPT
} function_type;

/// @brief The cellox compiler
typedef struct compiler_t {
    /// @brief The enclosing compiler
    /// @details This is needed to compile functions that are enclosed in another function or a script
    struct compiler_t * enclosing;
    /// @brief The main function
    object_function_t * function;
    /// @brief The type of the function that is currently executed
    function_type type;
    /// @brief The locals that were declared in the current scope
    local_t locals[UINT8_COUNT];
    // @brief The amount of local values that where declared in the current scope
    int32_t localCount;
    /// @brief The upvalues of the current scope (part of a closure)
    upvalue_t upvalues[UINT8_COUNT];
    /// @brief The scopedepth
    /// @details Used to determine whether a declared variable is a global or a local variable
    int32_t scopeDepth;
} compiler_t;

/// @brief  Class compiler struct definition
/// @details The class compiler is used to track the class we the compiler is currently processing.
typedef struct class_compiler_t {
    /// The enclosing class compiler structure
    struct class_compiler_t * enclosing;
    /// boolean value that determines whether a class has a superclass
    bool hasSuperclass;
} class_compiler_t;

/// @brief Maximum number of pending break/continue jump offsets within a single loop
#define LOOP_MAX_PENDING_JUMPS 256

/// @brief Tracks the context of the innermost loop being compiled
/// @details Chained via the enclosing pointer for nested loops
typedef struct loop_context_t {
    /// The enclosing loop context (NULL at the outermost loop)
    struct loop_context_t * enclosing;
    /// Backward jump target for continue (offset known at loop start); -1 for do-while (forward patches)
    int32_t continueTarget;
    /// Pending forward jump offsets for continue (do-while only)
    int32_t continueJumps[LOOP_MAX_PENDING_JUMPS];
    /// Number of pending continue jumps
    int32_t continueJumpCount;
    /// Pending forward jump offsets for break
    int32_t breakJumps[LOOP_MAX_PENDING_JUMPS];
    /// Number of pending break jumps
    int32_t breakJumpCount;
    /// Compiler local count at the time the loop was entered (for stack cleanup on break/continue)
    int32_t localCountAtLoop;
} loop_context_t;

#endif