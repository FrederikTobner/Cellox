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
 * @file dynamic_value_array.c
 * @brief File containing the dynamic value array implementation used internally by the compiler
 */
#include "dynamic_value_array.h"

#include <stdlib.h>

#include "../../backend/memory_mutator.h"

void dynamic_value_array_free(dynamic_value_array_t * array) {
    FREE_ARRAY(value_t, array->values, array->capacity);
    dynamic_value_array_init(array);
}

void dynamic_value_array_init(dynamic_value_array_t * array) {
    array->values = NULL;
    array->count = array->capacity = 0u;
}

void dynamic_value_array_remove(dynamic_value_array_t * array, size_t index) {
    if (index >= array->count) {
        return;
    }
    memcpy(array->values + index, array->values + index + 1, array->count - (index + 1));
    array->count--;
}

void dynamic_value_array_write(dynamic_value_array_t * array, value_t value) {
    if (array->capacity < array->count + 1u) {
        uint32_t oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        value_t * grownArray;
        grownArray = GROW_ARRAY(value_t, array->values, oldCapacity, array->capacity);
        array->values = grownArray;
    }
    array->values[array->count] = value;
    array->count++;
}
