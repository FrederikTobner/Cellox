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
 * @file chunk.h
 * @brief Header file containing the declarations of functionality regarding cellox chunks.
 */


#ifndef CELLOX_CHUNK_H_
#define CELLOX_CHUNK_H_

#include "common.h"
#include "dynamic_value_array.h"
#include "value.h"

/// @brief opcodes of the bytecode instruction set
enum opcode
{
    /// Pops the two most upper values from the stack, adds them and pushes the result onto the stack
    OP_ADD,
    /// Defines the arguments of the array literal declaration
    OP_ARRAY_LITERAL,
    /// Defines the arguments for the next function invocation
    OP_CALL,
    /// Defines a new class
    OP_CLASS,
    /// Defines a new closure for a function that is about to be called
    OP_CLOSURE,
    /// Closes all upvalus of the closure of the current function
    OP_CLOSE_UPVALUE,
    /// Defines a constant
    OP_CONSTANT,
    /// Defines a global variable
    OP_DEFINE_GLOBAL,
    /// Pops the two most upper values from the stack, divides the first with the second value and pushes the result on the stack
    OP_DIVIDE,
    /// Determines whether two the values on top of the are equal and  pushes the result on the stack
    OP_EQUAL,
    /// Pops the two most upper values from the stack, raises the first with the second value and pushes the result on the stack
    OP_EXPONENT,
    /// Pushes the boolean value false on the stack
    OP_FALSE,
    /// Gets the value of a global variable and stores it on the stack
    OP_GET_GLOBAL,
    /// Gets the value of a single character in a string at the specified index. Pushes the result on the stack
    OP_GET_INDEX_OF,
    /// Gets the value of a local variable and stores it on the stack
    OP_GET_LOCAL,
    /// Gets the value of the property of a Cellox object and stores it on the stack
    OP_GET_PROPERTY,
    /// Gets the two most upper values from the stack and uses them to narrow down a certain range that is used to create a slice from an array or a string
    OP_GET_SLICE_OF,
    /// Gets the parent class of a value and stores it on the stack
    OP_GET_SUPER,
    /// Gets the value of the upvalue and stores it on the stack
    OP_GET_UPVALUE,
    /// Pops the two most upper values from the stack, and pushes the value true on the stack if the first number is greater than the second number
    OP_GREATER,
    /// Adds another class as the parent to a class declaration
    OP_INHERIT,
    /// Invokes a function
    OP_INVOKE,
    /// Jumps from the current position to another position in the code, determined by a certain offset - used at the beginning of a loop, conditional statements
    OP_JUMP,
    /// Jumps if the value on top of the stack is false
    OP_JUMP_IF_FALSE,
    /// Pops the two most upper values from the stack, and pushes the value true on the stack if the first number is less than the second number
    OP_LESS,
    /// Jumps from the current position to another position in the code, determined by a certain offset - used at the end of a loop
    OP_LOOP,
    /// Calls a Method
    OP_METHOD,
    /// Pops the two most upper values from the stack, divides the first with the second value and pushes the remainder of the division onto the stack
    OP_MODULO,
    /// Pops the two most upper values from the stack adds them and pushes the result onto the stack
    OP_MULTIPLY,
    /// Negates the value on top of the stack
    OP_NEGATE,
    /// Converts the value on top of the stack from a truthy value to a falsy value and vice versa
    OP_NOT,
    /// Pushes a null value on the stack
    OP_NULL,
    /// Pops a value from the stack
    OP_POP,
    /// Returns the value that is stored on the top of the stack
    OP_RETURN,
    /// Sets the value of a global variable
    OP_SET_GLOBAL,
    /// Copies the value of a string and alters a single character at the specified index. Pushes the result on the stack.
    OP_SET_INDEX_OF,
    /// Sets the value of a local variable
    OP_SET_LOCAL,
    /// Sets the value of a property
    OP_SET_PROPERTY,
    /// Sets an upvalue that is captured by the current closure
    OP_SET_UPVALUE,
    /// Pops the two most upper values from the stack, subtracts the second value from the first value and pushes the result onto the stack
    OP_SUBTRACT,
    /// Invokes a method of the parent class
    OP_SUPER_INVOKE,
    /// Pushes the boolean value true on the stack
    OP_TRUE,
};

/// @brief Line info of a chunk
/// @details Stores the index of the last instruction in a line and the line number
typedef struct 
{
  /// The line number in the source code
  uint32_t lineNumber;
  /// The index of the last bytecode instruction corresponding to the line number
  uint32_t lastOpCodeIndexInLine;
} line_info_t;

/// @brief A dynamic array structure of bytecode instructions and constants
/// @details instructions are idealized instructions for an abstract/virtual computer. 
/// The constants in the program are defined at the beginning of the chunk.
typedef struct
{
    /// Amount of bytecode instructions in the chunk
    uint32_t byteCodeCount;
    /// Capacity of bytecode instructions of the chunk
    uint32_t byteCodeCapacity;
    /// Amount of line info in the chunk
    uint32_t lineInfoCount;
    /// Capacity for line info of the chunk
    uint32_t lineInfoCapacity;
    /// Operand Codes
    uint8_t * code;
    /// Stores line information for the bytecode instructions stored in the chunk
    line_info_t * lineInfos;
    /// Constants stored in the chunk
    dynamic_value_array_t constants;
} chunk_t;

/// @brief Adds a constant to the chunk
/// @param chunk The chunk where the value is added
/// @param value The value that is added
/// @return The index of the added constant
int32_t chunk_add_constant(chunk_t * chunk, value_t value);

/// @brief Determines the corresponding line number for a bytecode instruction by the index of the instruction in the chunk
/// @param chunk The chunk where the bytecode instruction is stored
/// @param opCodeIndex The index of the bytecode instruction 
/// @return The line number as an unnsigned 32-bit integer value 
uint32_t chunk_determine_line_by_index(chunk_t * chunk, uint32_t opCodeIndex);

/// @brief Deallocates the memory used by the chunk
/// @param chunk The chunk that is freed
void chunk_free(chunk_t * chunk);

/// @brief Initializes a chunk
/// @param chunk The chunk that is initialized
void chunk_init(chunk_t * chunk);

/// @brief 
/// @param chunk 
/// @param constantIndex 
void chunk_remove_constant(chunk_t * chunk, uint32_t constantIndex);


void chunk_remove_bytecode(chunk_t * chunk, uint32_t startIndex, uint32_t amount);

/// @brief Writes to a single bytecode instruction to a chunk
/// @param chunk The chunk where the byte is added
/// @param byte The value of the byte that is added to the chunk
/// @param line Line counter of the Token that corresponds to the generated bytecode
/// @details The line in the soucecode that corresponds to the bytecode instruction that is stored
void chunk_write(chunk_t * chunk, uint8_t byte, int32_t line);

#endif
