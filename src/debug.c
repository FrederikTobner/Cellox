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
 * @file debug.c
 * @brief File containing implementation of functions that are used to debug the interpreter.
 */

#include "debug.h"

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "object.h"
#include "value.h"

static int32_t debug_byte_instruction(char const *, chunk_t *, int32_t);
static int32_t debug_constant_instruction(char const *, chunk_t *, int32_t);
static int debug_invoke_instruction(char const *, chunk_t *, int32_t);
static int32_t debug_jump_instruction(char const *, int32_t, chunk_t *, int32_t);
static void debug_print_chunk_metadata(chunk_t *, char const *, uint32_t);
static int32_t debug_simple_instruction(char const *, int32_t);

void debug_disassemble_chunk(chunk_t * chunk, char const * name, uint32_t arity)
{
    debug_print_chunk_metadata(chunk, name, arity);
    for (int32_t offset = 0; offset < chunk->byteCodeCount;)
        offset = debug_disassemble_instruction(chunk, offset);
}

int32_t debug_disassemble_instruction(chunk_t * chunk, int32_t offset)
{
    printf("%04X ", offset);
    if (offset > 0 && chunk_determine_line_by_index(chunk, offset) == chunk_determine_line_by_index(chunk, offset - 1))
        printf("   | ");
    else
        printf("%4d ", chunk_determine_line_by_index(chunk, offset));
    // Instruction specific behaviour
    uint8_t instruction = chunk->code[offset];
    printf(" OP_%02X: ", instruction);
    switch (instruction)
    {
    case OP_ADD:
        return debug_simple_instruction("ADD", offset);
    case OP_CALL:
        return debug_byte_instruction("CALL", chunk, offset);
    case OP_CLASS:
        return debug_constant_instruction("CLASS", chunk, offset);
    case OP_CLOSURE:
    {
        offset++;
        uint8_t constant = chunk->code[offset++];
        printf("%-16s %04X ", "CLOSURE", constant);
        value_print(chunk->constants.values[constant]);
        printf("\n");
        object_function_t *function = AS_FUNCTION(chunk->constants.values[constant]);
        for (uint32_t j = 0; j < function->upvalueCount; j++)
        {
            int32_t isLocal = chunk->code[offset++];
            int32_t index = chunk->code[offset++];
            printf("%04X      |                     %s %d\n", offset - 2, isLocal ? "local" : "upvalue", index);
        }
        return offset;
    }
    case OP_CLOSE_UPVALUE:
        return debug_simple_instruction("CLOSE_UPVALUE", offset);
    case OP_CONSTANT:
        return debug_constant_instruction("CONSTANT", chunk, offset);
    case OP_DEFINE_GLOBAL:
        return debug_constant_instruction("DEFINE_GLOBAL", chunk, offset);
    case OP_DIVIDE:
        return debug_simple_instruction("DIVIDE", offset);
    case OP_ARRAY_LITERAL:
        return debug_byte_instruction("DYNAMIC_ARRAY_LITERAL", chunk, offset);
    case OP_EQUAL:
        return debug_simple_instruction("EQUAL", offset);
    case OP_EXPONENT:
        return debug_simple_instruction("EXPONENT", offset);
    case OP_FALSE:
        return debug_simple_instruction("FALSE", offset);
    case OP_GET_GLOBAL:
        return debug_constant_instruction("GET_GLOBAL", chunk, offset);
    case OP_GET_INDEX_OF:
        return debug_simple_instruction("GET_INDEX_OF", offset);
    case OP_GET_LOCAL:
        return debug_byte_instruction("GET_LOCAL", chunk, offset);
    case OP_GET_PROPERTY:
        return debug_constant_instruction("GET_PROPERTY", chunk, offset);
    case OP_GET_SLICE_OF:
        return debug_simple_instruction("GET_RANGE_OF", offset);
    case OP_GET_SUPER:
        return debug_constant_instruction("GET_SUPER", chunk, offset);
    case OP_GET_UPVALUE:
        return debug_byte_instruction("GET_UPVALUE", chunk, offset);
    case OP_GREATER:
        return debug_simple_instruction("GREATER", offset);
    case OP_INHERIT:
        return debug_simple_instruction("INHERIT", offset);
    case OP_INVOKE:
        return debug_invoke_instruction("INVOKE", chunk, offset);
    case OP_JUMP:
        return debug_jump_instruction("JUMP", 1, chunk, offset);
    case OP_JUMP_IF_FALSE:
        return debug_jump_instruction("JUMP_IF_FALSE", 1, chunk, offset);
    case OP_LESS:
        return debug_simple_instruction("LESS", offset);
    case OP_LOOP:
        return debug_jump_instruction("LOOP", -1, chunk, offset);
    case OP_METHOD:
        return debug_constant_instruction("METHOD", chunk, offset);
    case OP_MODULO:
        return debug_simple_instruction("MODULO", offset);
    case OP_MULTIPLY:
        return debug_simple_instruction("MULTIPLY", offset);
    case OP_NEGATE:
        return debug_simple_instruction("NEGATE", offset);
    case OP_NOT:
        return debug_simple_instruction("NOT", offset);
    case OP_NULL:
        return debug_simple_instruction("NULL", offset);
    case OP_POP:
        return debug_simple_instruction("POP", offset);
    case OP_RETURN:
        return debug_simple_instruction("RETURN", offset);
    case OP_SET_GLOBAL:
        return debug_constant_instruction("SET_GLOBAL", chunk, offset);
    case OP_SET_INDEX_OF:
        return debug_simple_instruction("SET INDEX OF", offset);
    case OP_SET_LOCAL:
        return debug_byte_instruction("SET_LOCAL", chunk, offset);
    case OP_SET_UPVALUE:
        return debug_byte_instruction("SET_UPVALUE", chunk, offset);
    case OP_SET_PROPERTY:
        return debug_constant_instruction("SET_PROPERTY", chunk, offset);
    case OP_SUBTRACT:
        return debug_simple_instruction("SUBTRACT", offset);
    case OP_SUPER_INVOKE:
        return debug_invoke_instruction("SUPER_INVOKE", chunk, offset);
    case OP_TRUE:
        return debug_simple_instruction("TRUE", offset);
    default:
        printf("Unknown opcode %02X\n", instruction);
        return offset + 1;
    }
}

/// @brief Shows the slot number of a local variable
/// @param name The name of the local variable
/// @param chunk The chunk where the local variable is stored
/// @param offset The offset of the local variable, used for getting the local variable
/// @return The index of the next bytecode instruction in the chunk
static int32_t debug_byte_instruction(char const * name, chunk_t * chunk, int32_t offset)
{
    uint8_t slot = *(chunk->code + offset + 1);
    printf("%-16s %04X\n", name, slot);
    return offset + 2;
}

/// @brief Dissasembles a constant instruction - OP_CONSTANT
/// @param name The name of the constant
/// @param chunk The chunk where the constant is located
/// @param offset The offset of the constant
/// @return The
static int32_t debug_constant_instruction(char const * name, chunk_t * chunk, int32_t offset)
{
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s %04X '", name, constant);
    value_print(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}

/// @brief Dissasembles a invoke instruction
/// @details This can either be a INVOKE or a SUPER_INVOKE Instruction
static int debug_invoke_instruction(char const * name, chunk_t *chunk, int32_t offset)
{
    uint8_t constant = *(chunk->code + offset + 1);
    uint8_t argCount = *(chunk->code + offset + 2);
    printf("%-16s (%d args) %04X '", name, argCount, constant);
    value_print(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 3;
}

/// Dissasembles a jump instruction (with a 16-bit operand)
static int32_t debug_jump_instruction(char const * name, int32_t sign, chunk_t * chunk, int32_t offset)
{
    uint16_t jump = (uint16_t)(*(chunk->code + offset + 1) << 8);
    jump |= *(chunk->code + offset + 2);
    printf("%-16s %04X -> %04X\n", name, offset, offset + 3 + sign * jump);
    return offset + 3;
}

/// @brief Provides additional metadata to a chunk and prints it to the stdandard output
/// @param chunk The chunk that is examined
/// @param name The name of the top level function of the chunk
/// @param arity The arity of the top level function of the chunk
static void debug_print_chunk_metadata(chunk_t * chunk, char const * name, uint32_t arity)
{
    uint32_t stringCount, numberCount, functionCount, classCount;
    stringCount = numberCount = functionCount = classCount = 0u;
    printf("function <%s> (%i bytes of bytecode at 0x%p)\n", name, chunk->byteCodeCount, chunk->code);
    for (size_t i = 0; i < chunk->constants.count; i++)
    {
        if (IS_OBJECT(chunk->constants.values[i]))
        {
            switch (OBJECT_TYPE(chunk->constants.values[i]))
            {
            case OBJECT_FUNCTION:
                functionCount++;
                break;
            default:
                break;
            }
        }
        else
        {
            numberCount++;
        }
    }
    for (uint32_t j = 0; j < chunk->byteCodeCount; j++)
    {
        switch (chunk->code[j])
        {
        case OP_CONSTANT:
            if(j + 1 == chunk->byteCodeCount)
                break;
            if(IS_STRING(chunk->constants.values[chunk->code[j + 1]]))
                stringCount++;
            break;
        case OP_CLASS:
            if(j + 1 == chunk->byteCodeCount)
                break;
            if(IS_STRING(chunk->constants.values[chunk->code[j + 1]]))
            classCount++;
        case OP_DEFINE_GLOBAL:
        case OP_GET_GLOBAL:
        case OP_GET_PROPERTY:
        case OP_GET_SUPER:
        case OP_METHOD:
        case OP_SET_GLOBAL:
        case OP_SET_PROPERTY:
        case OP_CALL:
        case OP_GET_LOCAL:
        case OP_GET_UPVALUE:
        case OP_SET_LOCAL:
        case OP_SET_UPVALUE:
            j++;
            break;
        case OP_INVOKE:
        case OP_JUMP:
            j += 2;
            break;

        default:
            break;
        }
    } 

    printf("%i %s, %i %s, %i %s, %i %s, %i %s\n",
            arity, arity == 1 ? "param" : "params", 
            stringCount, stringCount == 1 ? "string constant" : "string constants",
            numberCount, numberCount == 1 ? "numerical constant" : "numerical constants",
            functionCount, functionCount == 1 ? "function" : "functions",
            classCount, classCount == 1 ? "class" : "classes");
}

/// Dissasembles a simple instruction
static int32_t debug_simple_instruction(char const * name, int32_t offset)
{
    printf("%s\n", name);
    return offset + 1;
}
