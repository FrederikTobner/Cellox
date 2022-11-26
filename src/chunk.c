#include "chunk.h"

#include <stdlib.h>

#include "memory.h"
#include "virtual_machine.h"

static bool chunk_is_full(chunk_t * chunk);

int32_t chunk_add_constant(chunk_t * chunk, value_t value)
{
  virtual_machine_push(value);
  dynamic_value_array_write(&chunk->constants, value);
  virtual_machine_pop();
  return chunk->constants.count - 1;
}

void chunk_free(chunk_t * chunk)
{
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  FREE_ARRAY(uint32_t, chunk->lines, chunk->capacity);
  dynamic_value_array_free(&chunk->constants);
  chunk_init(chunk);
}

void chunk_init(chunk_t * chunk)
{
  chunk->count = chunk->capacity = 0;
  chunk->code = NULL;
  chunk->lines = NULL;
  dynamic_value_array_init(&chunk->constants);
}

void chunk_write(chunk_t * chunk, uint8_t byte, int32_t line)
{
  if (chunk_is_full(chunk))
  {
    // Stores the oldcapacity of the chunk so we know how much memory we have to allocate
    uint32_t oldCapacity = chunk->capacity;
    // Increases capacity
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    // Allocates bytecode array
    uint8_t * grownChunk = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    if(!grownChunk)
        exit(EXIT_CODE_SYSTEM_ERROR);    
    chunk->code = grownChunk;
    // Allocates line array
    chunk->lines = GROW_ARRAY(uint32_t, chunk->lines, oldCapacity, chunk->capacity);
    if(!chunk->lines)
      exit(EXIT_CODE_SYSTEM_ERROR);
  }
  // Writes the bytecode instruction to the chunk
  chunk->code[chunk->count] = byte;
/* 
 * Adds line info from the sourceCode in case a runtime error occurs, 
 * so we can show the line in case of an error and increases the counter of the chunk 
*/
  chunk->lines[chunk->count++] = line;  
}

/// @brief Determines whether a chunk is completely filled with bytecode instructions
/// @param chunk The chunk that is checked if it is already filled
/// @return True if the chunk is full, false if not
static bool chunk_is_full(chunk_t * chunk)
{
    return chunk->capacity < chunk->count + 1;
}
