#include "chunk_optimizer.h"

void chunk_optimizer_optimize_chunk(chunk_t * chunk)
{
    for (size_t i = 0; i < chunk->byteCodeCount; i++)
    {
        switch (chunk->code[i])
        {
        case OP_CONSTANT:
            if(i + 4 < chunk->byteCodeCount) {
                if(chunk->code[i + 3] == OP_CONSTANT)
                {
                    // Constant folding
                    switch (chunk->code[i + 2])
                    {
                    case OP_ADD:
                    case OP_DIVIDE:
                    case OP_MULTIPLY:
                    case OP_SUBTRACT:
                        // TODO: Handle string and boolean literals
                        if(IS_NUMBER(chunk->constants.values[chunk->code[i + 1]]) && IS_NUMBER(chunk->constants.values[chunk->code[i + 3]]))
                        {
                            // Remove OP_ADD, OP_CONSTANT and the constant index and replace the first constant at the index with the sum of the two constants
                            double propagatedResult;
                            switch (chunk->code[i + 2])
                            {
                                case OP_ADD:
                                    propagatedResult = AS_NUMBER(chunk->constants.values[chunk->code[i + 1]]) + 
                                                        AS_NUMBER(chunk->constants.values[chunk->code[i + 3]]);
                                    break;
                                case OP_DIVIDE:
                                    propagatedResult = AS_NUMBER(chunk->constants.values[chunk->code[i + 1]]) / 
                                                        AS_NUMBER(chunk->constants.values[chunk->code[i + 3]]);
                                    break;
                                case OP_MULTIPLY:
                                    propagatedResult = AS_NUMBER(chunk->constants.values[chunk->code[i + 1]]) * 
                                                        AS_NUMBER(chunk->constants.values[chunk->code[i + 3]]);
                                    break;
                                case OP_SUBTRACT:
                                    propagatedResult = AS_NUMBER(chunk->constants.values[chunk->code[i + 1]]) - 
                                                        AS_NUMBER(chunk->constants.values[chunk->code[i + 3]]);
                                    break;
                                default:
                                    break;
                            }
                            chunk->constants.values[chunk->code[i + 1]] = NUMBER_VAL(propagatedResult);
                            chunk_remove_constant(chunk, chunk->code[i + 3]);
                            chunk_remove_bytecode(chunk, i + 2, 3);
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