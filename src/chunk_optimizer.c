#include "chunk_optimizer.h"

static void chunk_optimizer_fold_numerical_expression(chunk_t *, int32_t *);

void chunk_optimizer_optimize_chunk(chunk_t * chunk)
{
    for (int32_t i = 0; i < chunk->byteCodeCount; i++)
    {
        switch (chunk->code[i])
        {
        case OP_CONSTANT:
            if(i + 4 < chunk->byteCodeCount) {
                if(chunk->code[i + 2] == OP_CONSTANT)
                {
                    // Constant folding
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
            i++;
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
            fooldedConsant = AS_NUMBER(chunk->constants.values[chunk->code[*indexPointer + 1]]) + 
                                AS_NUMBER(chunk->constants.values[chunk->code[*indexPointer + 3]]);
            break;
        case OP_DIVIDE:
            fooldedConsant = AS_NUMBER(chunk->constants.values[chunk->code[*indexPointer + 1]]) / 
                                AS_NUMBER(chunk->constants.values[chunk->code[*indexPointer + 3]]);
            break;
        case OP_MULTIPLY:
            fooldedConsant = AS_NUMBER(chunk->constants.values[chunk->code[*indexPointer + 1]]) * 
                                AS_NUMBER(chunk->constants.values[chunk->code[*indexPointer + 3]]);
            break;
        case OP_SUBTRACT:
            fooldedConsant = AS_NUMBER(chunk->constants.values[chunk->code[*indexPointer + 1]]) - 
                                AS_NUMBER(chunk->constants.values[chunk->code[*indexPointer + 3]]);
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
    chunk->constants.values[chunk->code[*indexPointer + 1]] = NUMBER_VAL(fooldedConsant);
    chunk_remove_constant(chunk, chunk->code[*indexPointer + 3]);
    chunk_remove_bytecode(chunk, *indexPointer + 2, 3);                            
    if(*indexPointer > 2 && chunk->code[*indexPointer - 2] == OP_CONSTANT)                                
        *indexPointer -= 4; // Checking prevoius bytecode instruction again for recursive constant folding 
}