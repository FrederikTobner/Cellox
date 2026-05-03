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
 * @file object.c
 * @brief File containing implementation of functionalitity regarding cellox objects.
 */

#include "object.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "backend/memory_mutator.h"
#include "string_utils.h"
#include "language-models/data-structures/value_hash_table.h"

static value_hash_table_t * vm_string_table = NULL;
static object_t ** vm_objects_head = NULL;
static void (*obj_gc_push)(value_t) = NULL;
static value_t (*obj_gc_pop)(void) = NULL;

void object_set_vm_string_table(value_hash_table_t * table) {
    vm_string_table = table;
}

void object_set_vm_objects(object_t ** objects) {
    vm_objects_head = objects;
}

void object_set_gc_guard_hooks(void (*push_fn)(value_t), value_t (*pop_fn)(void)) {
    obj_gc_push = push_fn;
    obj_gc_pop = pop_fn;
}

/// Marko for allocating a new object
#define ALLOCATE_OBJECT(type, objectType) (type *)object_allocate_object(sizeof(type), objectType)

/// The object types of cellox as a string
static char const * objectTypesStringified[] = {
    "array",          "method",        "error set", "error",    "result",     "instance",
    "class",          "closure",       "function",  "native function", "string", "upvalue",
    "unknown"};

static object_t * object_allocate_object(size_t, object_type);
static object_string_t * object_allocate_string(char *, uint32_t, uint32_t);
static void object_print_function(object_function_t *);

object_string_t * object_copy_string(char const * chars, uint32_t length, bool removeBackSlash) {
    uint32_t hash;
    object_string_t * interned;
    char * heapChars;

    if (!string_utils_contains_character_restricted(chars, '\\', length)) {
        hash = string_utils_hash_string(chars, length);
        if (vm_string_table) {
            interned = value_hash_table_find_string(vm_string_table, chars, length, hash);
            if (interned) {
                return interned;
            }
        }
        heapChars = ALLOCATE(char, length + 1);
        memcpy(heapChars, chars, length);
        heapChars[length] = '\0';
    } else {
        heapChars = ALLOCATE(char, length + 1);
        memcpy(heapChars, chars, length);
        heapChars[length] = '\0';
        char * next = NULL;
        for (uint32_t i = 0; i < length; i++) {
            if (heapChars[i] == '\\') {
                if (string_utils_resolve_escape_sequence(&heapChars[i], &length)) {
                    free(heapChars);
                    return NULL;
                }
            }
        }
        hash = string_utils_hash_string(heapChars, length);
        if (vm_string_table) {
            interned = value_hash_table_find_string(vm_string_table, heapChars, length, hash);
            if (interned) {
                free(heapChars);
                return interned;
            }
        }
    }

    return object_allocate_string(heapChars, strlen(heapChars), hash);
}

object_bound_method_t * object_new_bound_method(value_t receiver, object_closure_t * method) {
    object_bound_method_t * bound = ALLOCATE_OBJECT(object_bound_method_t, OBJECT_BOUND_METHOD);
    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

object_class_t * object_new_class(object_string_t * name) {
    object_class_t * celloxClass = ALLOCATE_OBJECT(object_class_t, OBJECT_CLASS);
    celloxClass->name = name;
    value_hash_table_init(&celloxClass->methods);
    return celloxClass;
}

object_error_set_t * object_new_error_set(object_string_t * name) {
    object_error_set_t * errorSet = ALLOCATE_OBJECT(object_error_set_t, OBJECT_ERROR_SET);
    errorSet->name = name;
    value_hash_table_init(&errorSet->variants);
    return errorSet;
}

object_error_value_t * object_new_error_value(object_error_set_t * errorSet, object_string_t * name) {
    object_error_value_t * errorValue = ALLOCATE_OBJECT(object_error_value_t, OBJECT_ERROR_VALUE);
    errorValue->errorSet = errorSet;
    errorValue->name = name;
    return errorValue;
}

object_result_t * object_new_result(value_t payload, bool isError) {
    object_result_t * result = ALLOCATE_OBJECT(object_result_t, OBJECT_RESULT);
    result->isError = isError;
    result->payload = payload;
    return result;
}

object_dynamic_value_array_t * object_new_dynamic_value_array(void) {
    object_dynamic_value_array_t * array = ALLOCATE_OBJECT(object_dynamic_value_array_t, OBJECT_ARRAY);
    dynamic_value_array_init(&array->array);
    return array;
}

object_closure_t * object_new_closure(object_function_t * function) {
    object_upvalue_t ** upvalues = ALLOCATE(object_upvalue_t *, function->upvalueCount);
    for (uint32_t i = 0; i < function->upvalueCount; i++) {
        upvalues[i] = NULL;
    }
    object_closure_t * closure = ALLOCATE_OBJECT(object_closure_t, OBJECT_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

object_function_t * object_new_function(void) {
    object_function_t * function = ALLOCATE_OBJECT(object_function_t, OBJECT_FUNCTION);
    function->arity = 0u;
    function->upvalueCount = 0u;
    function->name = NULL;
    chunk_init(&function->chunk);
    return function;
}

object_instance_t * object_new_instance(object_class_t * celloxClass) {
    object_instance_t * instance = ALLOCATE_OBJECT(object_instance_t, OBJECT_INSTANCE);
    instance->celloxClass = celloxClass;
    value_hash_table_init(&instance->fields);
    return instance;
}

object_native_t * object_new_native(native_function_t function, size_t arity) {
    object_native_t * native = ALLOCATE_OBJECT(object_native_t, OBJECT_NATIVE);
    native->function = function;
    native->arity = arity;
    return native;
}

object_upvalue_t * object_new_upvalue(value_t * slot) {
    // Allocating the memory used by the upvalue
    object_upvalue_t * upvalue = ALLOCATE_OBJECT(object_upvalue_t, OBJECT_UPVALUE);
    // We zero out the closed field of the upvalue when we create it
    upvalue->closed = NULL_VAL;
    // Adress of the slot where the closed over variables live (enclosing environment)
    upvalue->location = slot;
    // When we allocate a new upvalue, it is not attached to any list
    upvalue->next = NULL;
    return upvalue;
}

void object_print(value_t value) {
    switch (OBJECT_TYPE(value)) {
    case OBJECT_ARRAY:
        {
            object_dynamic_value_array_t * array = AS_ARRAY(value);
            putc('{', stdout);
            for (size_t i = 0; i < array->array.count; i++) {
                value_print(array->array.values[i]);
                if (i != array->array.count - 1) {
                    printf(", ");
                }
            }
            putc('}', stdout);
            break;
        }
    case OBJECT_BOUND_METHOD:
        object_print_function(AS_BOUND_METHOD(value)->method->function);
        break;
    case OBJECT_ERROR_SET:
        printf("<error set %s>", AS_ERROR_SET(value)->name->chars);
        break;
    case OBJECT_ERROR_VALUE:
        printf("%s.%s", AS_ERROR_VALUE(value)->errorSet->name->chars, AS_ERROR_VALUE(value)->name->chars);
        break;
    case OBJECT_RESULT:
        {
            object_result_t * result = AS_RESULT(value);
            if (result->isError) {
                printf("error(");
            } else {
                printf("ok(");
            }
            value_print(result->payload);
            putc(')', stdout);
            break;
        }
    case OBJECT_CLASS:
        printf("%s", AS_CLASS(value)->name->chars);
        break;
    case OBJECT_CLOSURE:
        object_print_function(AS_CLOSURE(value)->function);
        break;
    case OBJECT_FUNCTION:
        object_print_function(AS_FUNCTION(value));
        break;
    case OBJECT_INSTANCE:
        {
            object_instance_t * instance = AS_INSTANCE(value);
            if (!instance->fields.count) {
                printf("{}");
                break;
            }
            putc('{', stdout);
            value_t fieldValue;
            size_t fieldCounter = instance->fields.count;
            for (size_t i = 0; i < instance->fields.capacity; i++) {
                if (instance->fields.entries[i].key != NULL) {
                    printf("%s: ", instance->fields.entries[i].key->chars);
                    if (IS_STRING(instance->fields.entries[i].value)) {
                        putc('"', stdout);
                    }
                    value_print(instance->fields.entries[i].value);
                    if (IS_STRING(instance->fields.entries[i].value)) {
                        putc('"', stdout);
                    }
                    if (fieldCounter-- > 1) {
                        printf(", ");
                    }
                }
            }
            putc('}', stdout);
            break;
        }
    case OBJECT_NATIVE:
        printf("<native fn>");
        break;
    case OBJECT_STRING:
        printf("%s", AS_CSTRING(value));
        break;
    case OBJECT_UPVALUE:
        printf("upvalue");
        break;
    }
}

object_string_t * object_take_string(char * chars, uint32_t length) {
    uint32_t hash = string_utils_hash_string(chars, length);
    if (vm_string_table) {
        object_string_t * interned = value_hash_table_find_string(vm_string_table, chars, length, hash);
        if (interned) {
            FREE_ARRAY(char, chars, length + 1);
            return interned;
        }
    }
    return object_allocate_string(chars, length, hash);
}

/// @brief Creates a string allocates memory to store a string
/// @param chars Pointer to the start of the string
/// @param length The length of the string
/// @param hash The hashvalue of the string
/// @return The created string
static object_string_t * object_allocate_string(char * chars, uint32_t length, uint32_t hash) {
    object_string_t * string = ALLOCATE_OBJECT(object_string_t, OBJECT_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    if (obj_gc_push) {
        obj_gc_push(OBJECT_VAL(string));
    }
    // Adds the string to hashtable storing all the strings allocated by the virtualMachine
    if (vm_string_table) {
        value_hash_table_set(vm_string_table, string, NULL_VAL);
    }
    if (obj_gc_pop) {
        obj_gc_pop();
    }
    return string;
}

/// @brief Allocates the memory for an object of a given type
/// @param size The size of the object that is allocated
/// @param type The type of the allocated object
/// @return The allocated object
static object_t * object_allocate_object(size_t size, object_type type) {
    // Allocates the memory used by the Object
    object_t * object = (object_t *)memory_mutator_reallocate(NULL, 0, size);
    // Sets the type of the object
    object->type = type;
    // Disables mark so it is picked up by the Garbage Collection in the next cycle
    object->isMarked = false;
    // Adds the object at the start of the linked list storing the objects allocated by the virtualMachine
    if (vm_objects_head) {
        object->next = *vm_objects_head;
        *vm_objects_head = object;
    } else {
        object->next = NULL;
    }
#ifdef DEBUG_LOG_GC
    printf("%p allocated %zu bytes for %d\n", (void *)object, size, type);
#endif
    return object;
}

/// @brief Prints a function or a script
/// @param function The function that is printed
static void object_print_function(object_function_t * function) {
    if (!function->name) {
        // top level code
        printf("<script>");
        return;
    }
    // A function
    printf("<fun %s>", function->name->chars);
}

char const * object_stringify_type(object_t * object) {
    switch (object->type) {
    case OBJECT_ARRAY:
        return objectTypesStringified[0];
    case OBJECT_BOUND_METHOD:
        return objectTypesStringified[1];
    case OBJECT_ERROR_SET:
        return objectTypesStringified[2];
    case OBJECT_ERROR_VALUE:
        return objectTypesStringified[3];
    case OBJECT_RESULT:
        return objectTypesStringified[4];
    case OBJECT_INSTANCE:
        return ((object_instance_t *)object)->celloxClass->name->chars;
    case OBJECT_CLASS:
        return objectTypesStringified[6];
    case OBJECT_CLOSURE:
        return objectTypesStringified[7];
    case OBJECT_FUNCTION:
        return objectTypesStringified[8];
    case OBJECT_NATIVE:
        return objectTypesStringified[9];
    case OBJECT_STRING:
        return objectTypesStringified[10];
    case OBJECT_UPVALUE:
        return objectTypesStringified[11];
    default:
        return objectTypesStringified[12];
    }
}
