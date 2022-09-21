#include "chunk.h"

#include <stdlib.h>

#include "memory.h"
#include "virtual_machine.h"

static bool chunk_is_full(Chunk *chunk);

// Adds a constant to the chunk
int32_t chunk_add_constant(Chunk *chunk, Value value)
{
  vm_push(value);
  dynamic_array_write(&chunk->constants, value);
  vm_pop();
  return chunk->constants.count - 1;
}

// Free's a chunk (Deallocates the memory used by the chunk)
void chunk_free(Chunk *chunk)
{
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  FREE_ARRAY(uint32_t, chunk->lines, chunk->capacity);
  dynamic_array_free(&chunk->constants);
  chunk_init(chunk);
}

// Initializes a chunk
void chunk_init(Chunk *chunk)
{
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  chunk->lines = NULL;
  dynamic_array_init(&chunk->constants);
}

// Write to a already existing chunk
void chunk_write(Chunk *chunk, uint8_t byte, int32_t line)
{
  if (chunk_is_full(chunk))
  {
    // Stores the oldcapacity of the chunk so we know how much memory we have to allocate
    uint32_t oldCapacity = chunk->capacity;
    // Increases capacity
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    // Allocates bytecode array
    chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    // Allocates line array
    chunk->lines = GROW_ARRAY(uint32_t, chunk->lines, oldCapacity, chunk->capacity);
  }
  // Writes the bytecodeintstruction to the chunk
  chunk->code[chunk->count] = byte;
  // Adds line info from the sourceCode in case a runtime error occurs, so we can show the line in case of an error
  chunk->lines[chunk->count] = line;
  chunk->count++;
}

static bool chunk_is_full(Chunk *chunk)
{
    return chunk->capacity < chunk->count + 1;
}