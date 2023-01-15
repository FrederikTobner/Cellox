#include "chunk_optimizer.h"

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
                        // TODO: Handle string and boolean literals
                        if(IS_NUMBER(chunk->constants.values[chunk->code[i + 1]]) && IS_NUMBER(chunk->constants.values[chunk->code[i + 2]]))
                        {
                            // Removes OP_ADD, OP_CONSTANT and the constant index and replaced the first constant at the index with the result of evaluating the expression
                            double fooldedConsant;
                            switch (chunk->code[i + 4])
                            {
                                case OP_ADD:
                                    fooldedConsant = AS_NUMBER(chunk->constants.values[chunk->code[i + 1]]) + 
                                                        AS_NUMBER(chunk->constants.values[chunk->code[i + 3]]);
                                    break;
                                case OP_DIVIDE:
                                    fooldedConsant = AS_NUMBER(chunk->constants.values[chunk->code[i + 1]]) / 
                                                        AS_NUMBER(chunk->constants.values[chunk->code[i + 3]]);
                                    break;
                                case OP_MULTIPLY:
                                    fooldedConsant = AS_NUMBER(chunk->constants.values[chunk->code[i + 1]]) * 
                                                        AS_NUMBER(chunk->constants.values[chunk->code[i + 3]]);
                                    break;
                                case OP_SUBTRACT:
                                    fooldedConsant = AS_NUMBER(chunk->constants.values[chunk->code[i + 1]]) - 
                                                        AS_NUMBER(chunk->constants.values[chunk->code[i + 3]]);
                                    break;
                                default:
                                    #if defined(COMPILER_MSVC)
                                        // We assume this code to be unreachable.
                                        // This tells the optimizer that reaching default is undefiened behaviour 😨
                                        __assume(0);
                                    #else
                                        break;
                                    #endif
                            }
                            chunk->constants.values[chunk->code[i + 1]] = NUMBER_VAL(fooldedConsant);
                            chunk_remove_constant(chunk, chunk->code[i + 3]);
                            chunk_remove_bytecode(chunk, i + 2, 3);                            
                            if(i > 2 && chunk->code[i - 2] == OP_CONSTANT)                                
                                i -= 4; // Checking prevoius bytecode instruction again for recursive constant folding 
                        }                        
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