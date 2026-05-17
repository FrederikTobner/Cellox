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
 * @file object.h
 * @brief Header file containing the declarations of functionality regarding cellox objects.
 */

#ifndef CELLOX_OBJECT_H_
#define CELLOX_OBJECT_H_

#include "base/common.h"
#include "byte-code/chunk.h"
#include "language-models/value.h"
#include "language-models/value_hash_table.h"

/// Makro that determines the type of an object
#define OBJECT_TYPE(value)     (AS_OBJECT(value)->type)

/// Makro that determines if the object has the object type array
#define IS_ARRAY(value)        object_is_type(value, OBJECT_ARRAY)
///  Makro that determines if the object has the object type bound-method
#define IS_BOUND_METHOD(value) object_is_type(value, OBJECT_BOUND_METHOD)
/// Makro that determines if the object has the object type error set
#define IS_ERROR_SET(value)    object_is_type(value, OBJECT_ERROR_SET)
/// Makro that determines if the object has the object type error value
#define IS_ERROR_VALUE(value)  object_is_type(value, OBJECT_ERROR_VALUE)
/// Makro that determines if the object has the object type result wrapper
#define IS_RESULT(value)       object_is_type(value, OBJECT_RESULT)
///  Makro that determines if the object has the object type instance
#define IS_INSTANCE(value)     object_is_type(value, OBJECT_INSTANCE)
/// Makro that determines if the object has the object type class
#define IS_CLASS(value)        object_is_type(value, OBJECT_CLASS)
/// Makro that determines if the object has the object type closure
#define IS_CLOSURE(value)      object_is_type(value, OBJECT_CLOSURE)
/// Makro that determines if the object has the object type function
#define IS_FUNCTION(value)     object_is_type(value, OBJECT_FUNCTION)
/// Makro that determines if the object has is a native function
#define IS_NATIVE(value)       object_is_type(value, OBJECT_NATIVE)
/// Makro that determines if the object has the object type string
#define IS_STRING(value)       object_is_type(value, OBJECT_STRING)

/// Makro that gets the value of an object as a dynamic value array
#define AS_ARRAY(value)        ((object_dynamic_value_array_t *)AS_OBJECT(value))
/// Makro that gets the value of an object as a bound method
#define AS_BOUND_METHOD(value) ((object_bound_method_t *)AS_OBJECT(value))
/// Makro that gets the value of an object as an error set
#define AS_ERROR_SET(value)    ((object_error_set_t *)AS_OBJECT(value))
/// Makro that gets the value of an object as an error value
#define AS_ERROR_VALUE(value)  ((object_error_value_t *)AS_OBJECT(value))
/// Makro that gets the value of an object as a result wrapper
#define AS_RESULT(value)       ((object_result_t *)AS_OBJECT(value))
/// Makro that gets the value of an object as a cellox class instance
#define AS_INSTANCE(value)     ((object_instance_t *)AS_OBJECT(value))
/// Makro that gets the value of an object as a class
#define AS_CLASS(value)        ((object_class_t *)AS_OBJECT(value))
/// Makro that gets the value of an object as a closure
#define AS_CLOSURE(value)      ((object_closure_t *)AS_OBJECT(value))
/// Makro that gets the value of an object as a cstring
#define AS_CSTRING(value)      (((object_string_t *)AS_OBJECT(value))->chars)
/// Makro that gets the value of an object as a function
#define AS_FUNCTION(value)     ((object_function_t *)AS_OBJECT(value))
/// Makro that gets the value of an object as a native function
#define AS_NATIVE(value)       (((object_native_t *)AS_OBJECT(value))->function)
/// Makro that gets the value of an object as a string
#define AS_STRING(value)       ((object_string_t *)AS_OBJECT(value))

/// @brief Different type of objects
typedef enum {
    /// A dynamic array
    OBJECT_ARRAY,
    /// A method the is bound to an object
    OBJECT_BOUND_METHOD,
    /// An error set declaration
    OBJECT_ERROR_SET,
    /// A specific error value
    OBJECT_ERROR_VALUE,
    /// A result wrapper holding either success or error
    OBJECT_RESULT,
    /// A instance of a cellox class
    OBJECT_INSTANCE,
    /// A class in cellox
    OBJECT_CLASS,
    /// A closure
    OBJECT_CLOSURE,
    /// A cellox function
    OBJECT_FUNCTION,
    /// A native function
    OBJECT_NATIVE,
    /// A string
    OBJECT_STRING,
    /// An upvalue
    OBJECT_UPVALUE,
} object_type;

/// @brief A cellox object
struct object_t {
    /// The type of the object
    object_type type;
    /// Determines whether the object has already been marked by the grabage collector
    bool isMarked;
    /// pointer to the next object in the linear sequence of objects stored on the heap
    struct object_t * next;
};

/// @brief A cellox function
typedef struct {
    /// data that defines all types of objects
    object_t obj;
    /// The number of parametters a function has
    uint32_t arity;
    /// Number of values from enclosing scopes
    uint32_t upvalueCount;
    /// The instructions in the function
    chunk_t chunk;
    /// The name of the function
    object_string_t * name;
} object_function_t;

/// @brief A native function
typedef struct {
    /// data that defines all types of objects
    object_t obj;
    /// Reference to the native implementation in c
    native_function_t function;
    /// Expected arity (SIZE_MAX means variadic)
    size_t arity;
} object_native_t;

/// @brief ObjectString structure definition
struct object_string_t {
    /// data that defines all types of objects
    object_t obj;
    /// The length of the string
    uint32_t length;
    /// Pointer to the address in memory under that the string is stored
    char * chars;
    /// The hashValue of the string
    uint32_t hash;
};

/// @brief An object up-value structure (a local variable in an enclosing function)
typedef struct object_upvalue_t {
    /// data that defines all types of objects
    object_t obj;
    /// location of the upvalue in memory
    value_t * location;
    /// The Enclosed value after the current environment is left
    value_t closed;
    /// The memory location of the next upvalue in memory
    struct object_upvalue_t * next;
} object_upvalue_t;

/**
 * @brief Models a closure, also called lexical closure or function closure.
 * @details A closure is the combination of a function and references to its surrounding state).
 * The closures in cellox are based on the closures used by the LuaVM.
 * In other words, a closure gives you access to an outer function's scope from an inner function.
 * Closures only exist in languages with first class functions
 * and allow the function to access the values that are captured through it's surrounding state.
 */
typedef struct {
    /// data that defines all types of objects
    object_t obj;
    /// The function of the closure
    object_function_t * function;
    /// The upvalues which are captured by the closure
    object_upvalue_t ** upvalues;
    /// The amount of upvalues that is captured by the closure
    uint32_t upvalueCount;
} object_closure_t;

/// @brief A class structure - a class in cellox
typedef struct {
    /// data that defines all types of objects
    object_t obj;
    /// The name of the class
    object_string_t * name;
    /// The methods that are defined in the class body
    value_hash_table_t methods;
} object_class_t;

/// @brief A cellox class instance
typedef struct {
    /// data that defines all types of objects
    object_t obj;
    /// The class of the object instance
    object_class_t * celloxClass;
    /// The fields of the instance
    value_hash_table_t fields;
} object_instance_t;

/// @brief A bound method
typedef struct {
    /// data that defines all types of objects
    object_t obj;
    /// The value of the object the method is bound to
    value_t receiver;
    /// The closure of the method (The context of the cellox instance)
    object_closure_t * method;
} object_bound_method_t;

/// @brief A named error set
typedef struct {
    /// data that defines all types of objects
    object_t obj;
    /// The name of the error set
    object_string_t * name;
    /// The declared error variants stored as fields on the set
    value_hash_table_t variants;
} object_error_set_t;

/// @brief A concrete error value belonging to an error set
typedef struct {
    /// data that defines all types of objects
    object_t obj;
    /// The error set this value belongs to
    object_error_set_t * errorSet;
    /// The name of the concrete variant
    object_string_t * name;
} object_error_value_t;

/// @brief A fallible result wrapper
typedef struct {
    /// data that defines all types of objects
    object_t obj;
    /// Indicates whether payload stores an error or success value
    bool isError;
    /// The wrapped payload
    value_t payload;
} object_result_t;

/// @brief A dynamic array
typedef struct {
    /// data that defines all types of objects
    object_t obj;
    /// The underlying array
    dynamic_value_array_t array;
} object_dynamic_value_array_t;

/// @brief Copys the value of a string in the hashtable of the virtualMachine
/// @param chars Pointer to the character sequence / string
/// @param length The length of the character sequence
/// @param removeBackSlash Boolean value that determines whether backslashes should be resolved
/// @return The created string
/// @note If the string contains an unknown escape sequence NULL is instead returned to indicate the error
object_string_t * object_copy_string(char const * chars, uint32_t length, bool removeBackSlash);

/// @brief Creates a new method, that is bound to a closure
/// @param receiver The closure the method is bound to
/// @param method The method that is bound to the closure
/// @return The created method
object_bound_method_t * object_new_bound_method(value_t receiver, object_closure_t * method);

/// @brief Creates a new class in cellox
/// @param name The name of the class
/// @return The created class
object_class_t * object_new_class(object_string_t * name);

/// @brief Creates a new named error set
/// @param name The name of the error set
/// @return The created error set
object_error_set_t * object_new_error_set(object_string_t * name);

/// @brief Creates a new concrete error value
/// @param errorSet The set the value belongs to
/// @param name The concrete error name
/// @return The created error value
object_error_value_t * object_new_error_value(object_error_set_t * errorSet, object_string_t * name);

/// @brief Creates a new result wrapper
/// @param payload The wrapped payload
/// @param isError Indicates whether payload is an error
/// @return The created result wrapper
object_result_t * object_new_result(value_t payload, bool isError);

/// @brief Convenience macro: wrap a success value in a result
#define object_new_result_ok(val)    object_new_result((val), false)
/// @brief Convenience macro: wrap an error value in a result
#define object_new_result_error(val) object_new_result((val), true)

/// @brief Creates a new Closure
/// @param function The function that is used to create the upvalue
/// @return The created closure
object_closure_t * object_new_closure(object_function_t * function);

/// @brief Creates a new dynamic value array
/// @return The created array
object_dynamic_value_array_t * object_new_dynamic_value_array(void);

/// @brief Creates a new cellox function
/// @return The new function that was created
object_function_t * object_new_function(void);

/// @brief Creates a new cellox class instance
/// @param celloxClass The class of the instance
/// @return The new instance that was created
object_instance_t * object_new_instance(object_class_t * celloxClass);

/// @brief Creates a new native function object
/// @param function The native_function_t that is used to create the native function object
/// @param arity Expected arity (SIZE_MAX means variadic)
/// @return The new function that was created
object_native_t * object_new_native(native_function_t function, size_t arity);

/// @brief Creates a string or returns a string from the hashtable of the virtualMachine if it already exists
/// @param chars Pointer to the character sequence
/// @param length The length of the character sequence
/// @return The string that was created or found
object_string_t * object_take_string(char * chars, uint32_t length);

/// @brief Creates a new upvalue
/// @param slot The slot where the value will be placed
/// @return The upvalue that was created
object_upvalue_t * object_new_upvalue(value_t * slot);

/// @brief Prints the object
/// @param value The value that is printed
void object_print(value_t value);

/// @brief Gets the textual representation of a cellox type
/// @param object The object that is used
/// @return A character pointer that represents the type
char const * object_stringify_type(object_t * object);

/// @brief Registers the VM string intern table.
/// @details Called once from virtual_machine_init so that object_copy_string and
/// object_take_string can intern strings without depending on the VM header.
void object_set_vm_string_table(value_hash_table_t * table);

/// @brief Registers the VM objects list head pointer.
void object_set_vm_objects(object_t ** objects);

/// @brief Registers GC guard hooks (push/pop) so object allocation can protect new strings from GC.
void object_set_gc_guard_hooks(void (*push_fn)(value_t), value_t (*pop_fn)(void));

/// @brief Determines whether a value is of a given type
/// @param value The value that is checked
/// @param type The type that is used for checking the value
/// @return true is the value is of the given type, false if not
CLX_PURE CLX_ALWAYS_INLINE bool object_is_type(value_t value, object_type type) {
    return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

#endif
