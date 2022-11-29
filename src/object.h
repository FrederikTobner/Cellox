#ifndef CELLOX_OBJECT_H_
#define CELLOX_OBJECT_H_

#include "chunk.h"
#include "common.h"
#include "hash_table.h"
#include "native_functions.h"
#include "value.h"

/// Makro that determines the type of an object
#define OBJECT_TYPE(value) \
    (AS_OBJECT(value)->type)

///  Makro that determines if the object has the object type bound-method
#define IS_BOUND_METHOD(value) \
    object_is_type(value, OBJECT_BOUND_METHOD)
///  Makro that determines if the object has the object type instance
#define IS_INSTANCE(value) \
    object_is_type(value, OBJECT_INSTANCE)
/// Makro that determines if the object has the object type class
#define IS_CLASS(value) \
    object_is_type(value, OBJECT_CLASS)
/// Makro that determines if the object has the object type closure
#define IS_CLOSURE(value) \
    object_is_type(value, OBJECT_CLOSURE)
/// Makro that determines if the object has the object type function
#define IS_FUNCTION(value) \
    object_is_type(value, OBJECT_FUNCTION)
/// Makro that determines if the object has the object type native - native function
#define IS_NATIVE(value) \
    object_is_type(value, OBJECT_NATIVE)
/// Makro that determines if the object has the object type string
#define IS_STRING(value) \
    object_is_type(value, OBJECT_STRING)

/// Makro that gets the value of an object as a bound method
#define AS_BOUND_METHOD(value) \
    ((object_bound_method_t *)AS_OBJECT(value))
/// Makro that gets the value of an object as a cellox class instance
#define AS_INSTANCE(value) \
    ((object_instance_t *)AS_OBJECT(value))
/// Makro that gets the value of an object as a class
#define AS_CLASS(value) \
    ((object_class_t *)AS_OBJECT(value))
/// Makro that gets the value of an object as a closure
#define AS_CLOSURE(value) \
    ((object_closure_t *)AS_OBJECT(value))
/// Makro that gets the value of an object as a function
#define AS_FUNCTION(value) \
    ((object_function_t *)AS_OBJECT(value))
/// Makro that gets the value of an object as a native function
#define AS_NATIVE(value) \
    (((object_native_t *)AS_OBJECT(value))->function)
/// Makro that gets the value of an object as a string
#define AS_STRING(value) \
    ((object_string_t *)AS_OBJECT(value))
/// Makro that gets the value of an object as a cstring
#define AS_CSTRING(value) \
    (((object_string_t *)AS_OBJECT(value))->chars)

/// @brief Different type of objects
typedef enum
{
    /// A method the is bound to an object
    OBJECT_BOUND_METHOD,
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
} object_type_t;

/// @brief A cellox object
struct object_t
{
    /// The type of the object
    object_type_t type;
    /// Determines whether the object has already been marked by the grabage collector
    bool isMarked;
    /// pointer to the next object in the linear sequence of objects stored on the heap
    struct object_t * next;
};

/// @brief A cellox function
typedef struct
{
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
typedef struct
{
    /// data that defines all types of objects
    object_t obj;
    /// Reference to the native implementation in c
    native_function_t function;
} object_native_t;

/// @brief ObjectString structure definition
struct object_string_t
{
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
typedef struct object_upvalue_t
{
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
typedef struct
{
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
typedef struct
{
    /// data that defines all types of objects
    object_t obj;
    /// The name of the class
    object_string_t * name;
    /// The methods that are defined in the class body
    hash_table_t methods;
} object_class_t;

/// @brief A cellox class instance
typedef struct
{
    /// data that defines all types of objects
    object_t obj;
    /// The class of the object instance
    object_class_t * celloxClass;
    /// The fields of the instance
    hash_table_t fields;
} object_instance_t;

/// @brief A bound method
typedef struct
{
    /// data that defines all types of objects
    object_t obj;
    /// The value of the object the method is bound to
    value_t receiver;
    /// The closure of the method (The context of the cellox instance)
    object_closure_t * method;
} object_bound_method_t;

/// @brief Copys the value of a string in the hashtable of the virtualMachine
/// @param chars Pointer to the character sequence / string
/// @param length The length of the character sequence
/// @param removeBackSlash Boolean value that determines whether backslashes should be resolved
/// @return The created string
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

/// @brief Creates a new Closure
/// @param function The function that is used to create the upvalue
/// @return The created closure
object_closure_t * object_new_closure(object_function_t * function);

/// @brief Creates a new cellox function
/// @return The new function that was created
object_function_t * object_new_function();

/// @brief Creates a new cellox class instance
/// @param celloxClass The class of the instance
/// @return The new instance that was created
object_instance_t * object_new_instance(object_class_t * celloxClass);

/// @brief Creates a new native function object
/// @param function The native_function_t that is used to create the native function object
/// @return The new function that was created
object_native_t * object_new_native(native_function_t function);

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

/// @brief Determines whether a value is of a given type
/// @param value The value that is checked
/// @param type The type that is used for checking the value
/// @return true is the value is of the given type, false if not
static inline bool object_is_type(value_t value, object_type_t type)
{
    return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

#endif
