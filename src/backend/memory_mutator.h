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
 * @file memory_mutator.h
 * @brief Header file containing the declarations of functionality used for memory management.
 */

#ifndef CELLOX_MEMORY_MUTATOR_H_
#define CELLOX_MEMORY_MUTATOR_H_

#include "../common.h"
#include "../language-models/object.h"

/// Growth factor of a dynamic value array
#define ARRAY_GROWTH_FACTOR                 (1.5)

/// Growth factor of a hashtable
#define HASH_TABLE_GROWTH_FACTOR            (2u)

/// Makro that allocates the memory needed for a given type multiplied by the count
#define ALLOCATE(type, count)               ((type *)memory_mutator_reallocate(NULL, 0, sizeof(type) * (count)))

/// Makro that frees the memory used by a given type at the position specified by the pointer
#define FREE(type, pointer)                 (memory_mutator_reallocate(pointer, sizeof(type), 0))

/// Makro that dealocates an existing dynamic array
#define FREE_ARRAY(type, pointer, oldCount) (memory_mutator_reallocate(pointer, sizeof(type) * (oldCount), 0))

/// Makro that determines the increase in capacity for a dynamic array (initalizes capacity at 8)
#define GROW_CAPACITY(capacity)             ((capacity) < 8u ? 8u : (capacity)*ARRAY_GROWTH_FACTOR)

/// Makro that increases the size of a dynamic Array
#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    ((type *)memory_mutator_reallocate(pointer, sizeof(type) * (oldCount), sizeof(type) * (newCount)))

/// Determines the new size if a hashtable is grown
#define GROW_HASHTABLE_CAPACITY(capacity) ((capacity) < 8u ? 8u : (capacity)*HASH_TABLE_GROWTH_FACTOR)

/// @brief Dealocates the memory used by the objects of the virtualMachine
void memory_mutator_free_objects(void);

/// @brief Reallocates a block in memory
/// @param pointer Pointer to the memory block that is reallocated
/// @param oldSize The oldsize if the memory block
/// @param newSize The new size of the memory block
/// @return The reallocated memory block
void * memory_mutator_reallocate(void * pointer, size_t oldSize, size_t newSize);

/// @brief Dealocates the memory used by a single object
/// @param object The object that is freed
void memory_mutator_free_object(object_t * object);

#endif
