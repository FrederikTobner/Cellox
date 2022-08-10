#include "chunk.h"

#include <stdlib.h>

#include "memory.h"
#include "vm.h"

// Adds a constant to the chunk
int32_t addConstant(Chunk *chunk, Value value)
{
  push(value);
  writeValueArray(&chunk->constants, value);
  pop();
  return chunk->constants.count - 1;
}

// Free's a chunk (Deallocates the memory used by the chunk)
void freeChunk(Chunk *chunk)
{
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  FREE_ARRAY(int32_t, chunk->lines, chunk->capacity);
  freeValueArray(&chunk->constants);
  initChunk(chunk);
}

// Initializes a chunk
void initChunk(Chunk *chunk)
{
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  chunk->lines = NULL;
  initValueArray(&chunk->constants);
}

// Write to a already existing chunk
void writeChunk(Chunk *chunk, uint8_t byte, int32_t line)
{
  if (chunk->capacity < chunk->count + 1)
  {
    // Stores the oldcapacity of the chunk so we know how much memory we have to allocate
    int32_t oldCapacity = chunk->capacity;
    // Increases capacity
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    // Allocates bytecode array
    chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    // Allocates line array
    chunk->lines = GROW_ARRAY(int32_t, chunk->lines, oldCapacity, chunk->capacity);
  }
  // Writes the bytecodeintstruction to the chunk
  chunk->code[chunk->count] = byte;
  // Adds line info from the sourceCode in case a runtime error occurs, so we can show the line
  chunk->lines[chunk->count] = line;
  chunk->count++;
}