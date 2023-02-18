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
 * @file memory_mutator.c
 * @brief File containing implementation of functionality used for memory management.
 */

#include "memory_mutator.h"

#include <stdio.h>
#include <stdlib.h>

#include "garbage_collector.h"
#include "virtual_machine.h"

void memory_mutator_free_objects() {
    object_t * object = virtualMachine.objects;
    while (object) {
        object_t * next = object->next;
        memory_mutator_free_object(object);
        object = next;
    }
    if (virtualMachine.grayStack) {
        free(virtualMachine.grayStack);
    }
    virtualMachine.objects = NULL;
}

void * memory_mutator_reallocate(void * pointer, size_t oldSize, size_t newSize) {
    virtualMachine.bytesAllocated += newSize - oldSize;
    if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
        memory_collect_garbage();
#endif
        if (virtualMachine.bytesAllocated > virtualMachine.nextGC) {
            garbage_collector_collect_garbage();
        }
    }
    if (!newSize) {
        free(pointer);
        return NULL;
    }

    void * result = realloc(pointer, newSize);
    if (!result) {
        fprintf(stderr, "Failed too allocate memory");
        exit(EXIT_CODE_SYSTEM_ERROR);
    }
    return result;
}

/// @brief Dealocates the memomory used by the object
/// @param object The object that is freed
void memory_mutator_free_object(object_t * object) {
#ifdef DEBUG_LOG_GC
    printf("freed object %p of the type %s\n", (void *)object, object_stringify_type(object));
#endif
    switch (object->type) {
    case OBJECT_ARRAY:
        {
            object_dynamic_value_array_t * array = (object_dynamic_value_array_t *)object;
            dynamic_value_array_free(&array->array);
            FREE(object_dynamic_value_array_t, object);
            break;
        }
    case OBJECT_BOUND_METHOD:
        FREE(object_bound_method_t, object);
        break;
    case OBJECT_CLASS:
        {
            object_class_t * celloxClass = (object_class_t *)object;
            // If a class is unreachable, all the methods are unreachable, too.
            value_hash_table_free(&celloxClass->methods);
            FREE(object_class_t, object);
            break;
        }
    case OBJECT_CLOSURE:
        {
            object_closure_t * closure = (object_closure_t *)object;
            // If a closure is unreachable we also need to free all the memory used by the upvalues that are captured by the closure
            FREE_ARRAY(object_upvalue_t *, closure->upvalues, closure->upvalueCount);
            FREE(object_closure_t, object);
            break;
        }
    case OBJECT_FUNCTION:
        {
            object_function_t * function = (object_function_t *)object;
            // If a function is unreachable we also need to free all the memory used by the chunk
            chunk_free(&function->chunk);
            FREE(object_function_t, object);
            break;
        }
    case OBJECT_INSTANCE:
        {
            object_instance_t * instance = (object_instance_t *)object;
            // If a instance is unreachable we also need to free all the memory used by the fields
            value_hash_table_free(&instance->fields);
            FREE(object_instance_t, object);
            break;
        }
    case OBJECT_NATIVE:
        FREE(object_native_t, object);
        break;
    case OBJECT_STRING:
        {
            object_string_t * string = (object_string_t *)object;
            // If a string is unreachable we need to free the memory the underlying character sequence occupies
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(object_string_t, object);
            break;
        }
    case OBJECT_UPVALUE:
        FREE(object_upvalue_t, object);
        break;
    }
}
