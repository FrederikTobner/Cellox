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
 * @file chunk_optimizer.c
 * @brief File containing the implementation of functionality regarding the optimization of cellox chunks.
 */

#include "chunk_optimizer.h"

#define FOLD_EXPRESSION(op)         \
chunk->constants.values[chunk->code[*indexPointer + 1]] = \
                                NUMBER_VAL(AS_NUMBER(chunk->constants.values[chunk->code[*indexPointer + 1]]) op \
                                AS_NUMBER(chunk->constants.values[chunk->code[*indexPointer + 3]]))

static void chunk_optimizer_fold_numerical_expression(chunk_t *, int32_t *);

void chunk_optimizer_optimize_chunk(chunk_t * chunk)
{
    for (int32_t i = 0; i < chunk->byteCodeCount; i++)
    {
        switch (chunk->code[i])
        {
        case OP_CONSTANT:
            if (i + 4 < chunk->byteCodeCount) {
                if (chunk->code[i + 2] == OP_CONSTANT)
                {
                    // Constant folding 🙏
                    switch (chunk->code[i + 4])
                    {
                    case OP_ADD:
                    case OP_DIVIDE:
                    case OP_MULTIPLY:
                    case OP_SUBTRACT:
                        if(IS_NUMBER(chunk->constants.values[chunk->code[i + 1]]) && IS_NUMBER(chunk->constants.values[chunk->code[i + 2]]))
                            chunk_optimizer_fold_numerical_expression(chunk, &i);                       
                        break;                    
                    default:                        
                        break;
                    }
                }
            }
        case OP_ARRAY_LITERAL:
        case OP_CLASS:
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
            i++;
            break;
        case OP_INVOKE:
        case OP_JUMP:
            i += 2;
            break;
        
        default:
            break;
        }          
    }
}

/// @brief Folds a expression in a chunk
/// @param chunk The chunk where the expression is folded
/// @param indexPointer Pointer to the index of the first constant in the expression that is folded
static void chunk_optimizer_fold_numerical_expression(chunk_t * chunk, int32_t * indexPointer)
{
    // Removes OP_ADD, OP_CONSTANT and the constant index and replaced the first constant at the index with the result of evaluating the expression
    double fooldedConsant;   

    switch (chunk->code[*indexPointer + 4])
    {   
        case OP_ADD:
            FOLD_EXPRESSION(+);
            break;
        case OP_DIVIDE:
            FOLD_EXPRESSION(/);
            break;
        case OP_MULTIPLY:
            FOLD_EXPRESSION(*);
            break;
        case OP_SUBTRACT:
            FOLD_EXPRESSION(-);
            break;
        default:
            // We assume this code to be unreachable.
            #if defined(COMPILER_MSVC)                
                __assume(0);
            #elif defined(COMPILER_GCC) || defined(COMPILER_CLANG)
                __builtin_unreachable();   
            #else
                break;
            #endif
    }
    // Removing OP_CONSTANT and OP_ADD / OP_DIVIDE / OP_MULTIPLY / OP_SUBTRACT from the chunk
    chunk_remove_bytecode(chunk, *indexPointer + 2, 3);                            
    if (*indexPointer > 2 && chunk->code[*indexPointer - 2] == OP_CONSTANT)                                
        *indexPointer -= 4; // Checking prevoius bytecode instruction again for recursive constant folding 
}
