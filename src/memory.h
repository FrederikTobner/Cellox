#ifndef clox_memory_h

#define clox_memory_h

#include "common.h"
#include "object.h"

// Allocates the memory needed for a given type multiplied by the count
#define ALLOCATE(type, count) \
    (type *)reallocate(NULL, 0, sizeof(type) * (count))

// Frees the memory used by a given type at the position specified by the pointer
#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

// Determines the increase in capacity for a dynamic array (initalizes capacity at 8)
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) + 2)

// Grows an dynamic Array
#define GROW_ARRAY(type, pointer, oldCount, newCount)      \
    (type *)reallocate(pointer, sizeof(type) * (oldCount), \
                       sizeof(type) * (newCount))

// Dealocates an existing dynamic array
#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

/* Starts the garbage collection process.
 * The garbage collector of cellox is a precise GC.
 * That means that the garbage collector knows whether words in memory are pointers
 * and which store a value - like a string, boolean or a number.
 * The Algorithm that is used is called mark-and-sweep.
 * It consists of two phases:
 * 1. Mark phase
 * 2. Sweep phase
 * In the marking phase we start at the roots and traverse through all the objects the roots refer to.
 * In the sweeping phase all the reachable objects have been marked, and therefore we can reclaim the memory that is used by the unmarked objects.
 */
void collectGarbage();

// Dealocates the memory used by the objects of the vm
void freeObjects();

// Marks a cellox object
void markObject(Object *object);

// Marks a cellox value
void markValue(Value value);

// Reallocates the memory usage from a given pointer
void *reallocate(void *pointer, size_t oldSize, size_t newSize);

#endif