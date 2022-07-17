#ifndef clox_memory_h

#define clox_memory_h

#include "common.h"
#include "object.h"

// Allocates the memory needed for a given type multiplied by the count
#define ALLOCATE(type, count) \
    (type *)reallocate(NULL, 0, sizeof(type) * (count))

// Frees the memory used by a given type at the position specified by the pointer
#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

// Increases Capacity (initalizes capacity at 8)
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) + 2)

// Grows an dynamic Array
#define GROW_ARRAY(type, pointer, oldCount, newCount)      \
    (type *)reallocate(pointer, sizeof(type) * (oldCount), \
                       sizeof(type) * (newCount))

// Dealocates an existing dynamic array
#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

// Reallocates the memory usage from a given pointer
void *reallocate(void *pointer, size_t oldSize, size_t newSize);

#endif