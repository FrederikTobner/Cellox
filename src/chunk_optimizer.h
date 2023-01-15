#ifndef CELLOX_OPTIMIZER_H_
#define CELLOX_OPTIMIZER_H_

#include "chunk.h"

/// @brief Optimizes the chunk by using different compiler optimization techniques  
/// @param chunk The chunk that is optimized
/// @details At the moment only constant folding is used
void chunk_optimizer_optimize_chunk(chunk_t * chunk);

#endif