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
 * See the <https://www.gnu.org/licenses/gpl-3.0.html/>GNU General Public   *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file virtual_machine.h
 * @brief Header file containing of declarations of the virtual machine.
 * @details The virtual machine is stackbased.
 */

#ifndef CELLOX_VIRTUAL_MACHINE_H_
#define CELLOX_VIRTUAL_MACHINE_H_

#include "../language-models/data-structures/value_hash_table.h"
#include "../language-models/object.h"

/// @brief Maximum amount of frames the virtual machine can hold
/// @details The maxiimum depth of the callstack
#define FRAMES_MAX 64

/// @brief Maximum amount values that can be allocated on the stack of the VirtualMachine
/// @details These are currently 16384 values
#define STACK_MAX  (FRAMES_MAX * UINT8_COUNT)

/// @brief A call frame structure
/// @details This represents a single ongoing function call
typedef struct {
    /// The closure of the callframe
    object_closure_t * closure;
    /// The instruction pointer in the callframe
    uint8_t * ip;
    /// Points to the first slot in the stack of the virtualMachine the function can use
    value_t * slots;
} call_frame_t;

/// @brief A virtual machine
/// @details The processbased virtual machine that is used by the cellox compiler is a stackbased virtual machine
typedef struct {
    /// Callstack of the virtual machine
    call_frame_t callStack[FRAMES_MAX];
    /// The amount of callframes the virtualMachine currently holds
    uint32_t frameCount;
    /// Amount of objects in the virtual machine that are marked as gray -> objects that are already discovered but
    /// haven't been processed yet
    uint32_t grayCount;
    /// The capacity of the dynamic array storing the objects that were marked as gray
    uint32_t grayCapacity;
    /// Stack of the virtualMachine
    value_t stack[STACK_MAX];
    /// Pointer to the top of the stack
    value_t * stackTop;
    /// Hashtable that contains the global variables
    value_hash_table_t globals;
    /// Hashtable that contains the strings
    value_hash_table_t strings;
    /// String "init" used to look up the initializer of a class - reused for every init call
    object_string_t * initString;
    /// Upvalues of the closures of all the functions on the callstack
    object_upvalue_t * openUpvalues;
    /// Number of bytes that have been allocated by the virtualMachine
    size_t bytesAllocated;
    /// A treshhold when the next garbage Collection shall be triggered (e.g. a Megabyte)
    size_t nextGC;
    /// The objects that are allocated in the memory of the virtualMachine
    object_t * objects;
    /// The stack that contains all the gray objects
    object_t ** grayStack;
    /// The source code of the program
    char * program;
} virtual_machine_t;

/// @brief Result of the interpretation (sucessfull, error during compilation or at runtime)
typedef enum {
    /// No error occured
    INTERPRET_OK,
    /// Error that occured during the compilation process
    INTERPRET_COMPILE_ERROR,
    /// Error that occured at runtime
    INTERPRET_RUNTIME_ERROR,
} interpret_result;

extern virtual_machine_t virtualMachine;

/// Deallocates the memory used by the virtual machine
void virtual_machine_free(void);

/// Initializes the virtual machine
void virtual_machine_init(void);

/// @brief Interprets a lox program
/// @param program The source that is interpreted
/// @param freeProgram Boolean value that determines whether the sourcecode is freed by the virtual machine
/// @return A interpret result that indicates whether the program execution sucessful
interpret_result virtual_machine_interpret(char * program, bool freeProgram);

interpret_result virtual_machine_run_chunk(chunk_t chunk);

/// @brief Pushes a new Value on the stack
/// @param value The value that is pushed on the stack
void virtual_machine_push(value_t value);

/// @brief Pops a value from the stack
/// @return The value that was popped from the stack
value_t virtual_machine_pop(void);

#endif
