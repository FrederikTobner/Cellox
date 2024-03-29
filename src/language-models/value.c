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
 * @file value.c
 * @brief File containing implementation of functionalitity of cellox values.
 */

#include "value.h"

#include <stdio.h>

#include "object.h"

/// The value types of cellox represented sequence of characters
static char const * valueTypesStringified[] = {
    "boolean",
    "undefiened",
    "numerical",
};

void value_print(value_t value) {
#ifdef NAN_BOXING
    if (IS_BOOL(value)) {
        printf(AS_BOOL(value) ? "true" : "false");
    } else if (IS_NULL(value)) {
        printf("null");
    } else if (IS_NUMBER(value)) {
        printf("%g", AS_NUMBER(value));
    } else if (IS_OBJECT(value)) {
        object_print(value);
    }

#else
    switch (value.type) {
    case VAL_BOOL:
        printf(AS_BOOL(value) ? "true" : "false");
        break;
    case VAL_NULL:
        printf("null");
        break;
    case VAL_NUMBER:
        printf("%g", AS_NUMBER(value));
        break;
    case VAL_OBJ:
        object_print(value);
        break;
    }
#endif
}

char const * value_stringify_type(value_t value) {
#ifdef NAN_BOXING
    if (IS_BOOL(value)) {
        return valueTypesStringified[0];
    }
    if (IS_NULL(value)) {
        return valueTypesStringified[1];
    }
    if (IS_NUMBER(value)) {
        return valueTypesStringified[2];
    }
#else
    switch (value.type) {
    case VAL_BOOL:
        return valueTypesStringified[0];
    case VAL_NULL:
        return valueTypesStringified[1];
    case VAL_NUMBER:
        return valueTypesStringified[2];
    }
#endif
    return object_stringify_type(AS_OBJECT(value));
}

bool value_values_equal(value_t a, value_t b) {
#ifdef NAN_BOXING
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return AS_NUMBER(a) == AS_NUMBER(b);
    } else if (IS_ARRAY(a) && IS_ARRAY(b)) {
        object_dynamic_value_array_t * firstArray = AS_ARRAY(a);
        object_dynamic_value_array_t * secondArray = AS_ARRAY(b);
        if (firstArray->array.count != secondArray->array.count) {
            return false;
        }
        for (size_t i = 0; i < firstArray->array.count; i++) {
            if (!value_values_equal(firstArray->array.values[i], secondArray->array.values[i])) {
                return false;
            }
        }
        return true;
    }
    return a == b;
#else
    if (a.type != b.type) {
        return false;
    }
    switch (a.type) {
    case VAL_BOOL:
        return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NULL:
        return true;
    case VAL_NUMBER:
        return AS_NUMBER(a) == AS_NUMBER(b);
    case VAL_OBJ:
        if (IS_ARRAY(a) && IS_ARRAY(b)) {
            object_dynamic_value_array_t * firstArray = AS_ARRAY(a);
            object_dynamic_value_array_t * secondArray = AS_ARRAY(b);
            if (firstArray->array.count != secondArray->array.count) {
                return false;
            }
            for (size_t i = 0; i < firstArray->array.count; i++) {
                if (!value_values_equal(firstArray->array.values[i], secondArray->array.values[i])) {
                    return false;
                }
            }
            return true;
        }
        return AS_OBJECT(a) == AS_OBJECT(b);
    default:
        return false; // Unreachable.
    }
#endif
}
