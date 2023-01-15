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
 * @file chunk.c
 * @brief File containing the implementation of functionality regarding cellox chunks.
 */

#include "chunk.h"

#include <stdlib.h>

#include "memory_mutator.h"
#include "virtual_machine.h"

static void chunk_adjust_line_info_by_index(chunk_t *, uint32_t, int32_t);
static inline bool chunk_byte_code_is_full(chunk_t *);
static inline bool chunk_line_info_is_full(chunk_t *);

int32_t chunk_add_constant(chunk_t * chunk, value_t value)
{
  virtual_machine_push(value);
  dynamic_value_array_write(&chunk->constants, value);
  virtual_machine_pop();
  return chunk->constants.count - 1;
}

uint32_t chunk_determine_line_by_index(chunk_t * chunk, uint32_t opCodeIndex)
{
  line_info_t * upperBound = chunk->lineInfos + chunk->lineInfoCount;
  for (line_info_t * lip = chunk->lineInfos; lip < upperBound; lip++)
    if(lip->lastOpCodeIndexInLine >= opCodeIndex)
      return lip->lineNumber;
  exit(EXIT_CODE_COMPILATION_ERROR);  // Should be unreachable
}

void chunk_free(chunk_t * chunk)
{
  FREE_ARRAY(uint8_t, chunk->code, chunk->byteCodeCapacity);
  FREE_ARRAY(line_info_t, chunk->lineInfos, chunk->lineInfoCapacity);
  dynamic_value_array_free(&chunk->constants);
  chunk_init(chunk);
}

void chunk_init(chunk_t * chunk)
{
  chunk->byteCodeCount = chunk->byteCodeCapacity = chunk->lineInfoCount = chunk->lineInfoCapacity = 0;
  chunk->code = NULL;
  chunk->lineInfos = NULL;
  dynamic_value_array_init(&chunk->constants);
}

void chunk_remove_constant(chunk_t * chunk, uint32_t constantIndex) 
{
    dynamic_value_array_remove(&chunk->constants, constantIndex);
}

void chunk_remove_bytecode(chunk_t * chunk, uint32_t startIndex, uint32_t amount) 
{
    if (startIndex + amount >= chunk->byteCodeCount)
      return;
    memcpy((chunk->code + startIndex), (chunk->code + startIndex + amount), chunk->byteCodeCount - (startIndex + amount));
    chunk->byteCodeCount -= amount;
    chunk_adjust_line_info_by_index(chunk, startIndex, -(int32_t)amount);    
}

void chunk_write(chunk_t * chunk, uint8_t byte, int32_t line)
{
  if (chunk_byte_code_is_full(chunk))
  {
    // Stores the oldcapacity of the chunk so we know how much memory we have to allocate
    uint32_t oldCapacity = chunk->byteCodeCapacity;
    // Increases capacity
    chunk->byteCodeCapacity = GROW_CAPACITY(oldCapacity);
    // Allocates bytecode array
    uint8_t * grownChunk = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->byteCodeCapacity);
    if(!grownChunk)
        exit(EXIT_CODE_SYSTEM_ERROR);    
    chunk->code = grownChunk;
  }
  // Writes the bytecode instruction to the chunk
  chunk->code[chunk->byteCodeCount] = byte;

/* 
 * Adds line info from the sourceCode in case a runtime error occurs, 
 * so we can show the line in case of an error
 */
  if(chunk->lineInfoCount == 0 || chunk->lineInfos[chunk->lineInfoCount - 1].lineNumber != line) 
  {
      // Initialize line info
    if(chunk_line_info_is_full(chunk))
    {
      uint32_t oldCapacity = chunk->lineInfoCapacity;
      chunk->lineInfoCapacity = GROW_CAPACITY(oldCapacity);
      chunk->lineInfos = GROW_ARRAY(line_info_t, chunk->lineInfos, oldCapacity, chunk->lineInfoCapacity);
      if(!chunk->lineInfos)
        exit(EXIT_CODE_SYSTEM_ERROR);
    }
    chunk->lineInfos[chunk->lineInfoCount].lastOpCodeIndexInLine = chunk->byteCodeCount;
    chunk->lineInfos[chunk->lineInfoCount].lineNumber = line;
    chunk->lineInfoCount++;    
  }
  else 
  {
    chunk->lineInfos[chunk->lineInfoCount - 1].lastOpCodeIndexInLine = chunk->byteCodeCount;
  }
  chunk->byteCodeCount++;  
}

static void chunk_adjust_line_info_by_index(chunk_t * chunk, uint32_t opCodeIndex,  int32_t adjustment)
{
  line_info_t * upperBound = chunk->lineInfos + chunk->lineInfoCount;
  for (line_info_t * lip = chunk->lineInfos; lip < upperBound; lip++)
    if(lip->lastOpCodeIndexInLine >= opCodeIndex)
      lip->lastOpCodeIndexInLine += adjustment;
}

/// @brief Determines whether a chunk is completely filled with bytecode instructions
/// @param chunk The chunk that is checked if it is already filled
/// @return True if the chunk is full, false if not
static inline bool chunk_byte_code_is_full(chunk_t * chunk)
{
    return chunk->byteCodeCapacity < chunk->byteCodeCount + 1;
}

/// @brief Determines whether a chunk is completely filled with bytecode instructions
/// @param chunk The chunk that is checked if it is already filled
/// @return True if the chunk is full, false if not
static inline bool chunk_line_info_is_full(chunk_t * chunk)
{
    return chunk->lineInfoCapacity < chunk->lineInfoCount + 1;
}
