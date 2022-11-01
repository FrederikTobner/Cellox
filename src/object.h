#ifndef cellox_object_h
#define cellox_object_h

#include "chunk.h"
#include "common.h"
#include "table.h"
#include "value.h"

// Makro that determines the type of an object
#define OBJECT_TYPE(value) \
    (AS_OBJECT(value)->type)

//  Makro that determines if the object has the object type bound-method
#define IS_BOUND_METHOD(value) \
    object_is_type(value, OBJECT_BOUND_METHOD)
//  Makro that determines if the object has the object type instance
#define IS_INSTANCE(value) \
    object_is_type(value, OBJECT_INSTANCE)
// Makro that determines if the object has the object type class
#define IS_CLASS(value) \
    object_is_type(value, OBJECT_CLASS)
// Makro that determines if the object has the object type closure
#define IS_CLOSURE(value) \
    object_is_type(value, OBJECT_CLOSURE)
// Makro that determines if the object has the object type function
#define IS_FUNCTION(value) \
    object_is_type(value, OBJECT_FUNCTION)
// Makro that determines if the object has the object type native - native function
#define IS_NATIVE(value) \
    object_is_type(value, OBJECT_NATIVE)
// Makro that determines if the object has the object type string
#define IS_STRING(value) \
    object_is_type(value, OBJECT_STRING)

//// Makro that gets the value of an object as a bound method
#define AS_BOUND_METHOD(value) \
    ((object_bound_method_t *)AS_OBJECT(value))
// Makro that gets the value of an object as a cellox class instance
#define AS_INSTANCE(value) \
    ((object_instance_t *)AS_OBJECT(value))
// Makro that gets the value of an object as a class
#define AS_CLASS(value) \
    ((object_class_t *)AS_OBJECT(value))
// Makro that gets the value of an object as a closure
#define AS_CLOSURE(value) \
    ((object_closure_t *)AS_OBJECT(value))
// Makro that gets the value of an object as a function
#define AS_FUNCTION(value) \
    ((object_function_t *)AS_OBJECT(value))
// Makro that gets the value of an object as a native function
#define AS_NATIVE(value) \
    (((object_native_t *)AS_OBJECT(value))->function)
// Makro that gets the value of an object as a string
#define AS_STRING(value) \
    ((object_string_t *)AS_OBJECT(value))
// Makro that gets the value of an object as a cstring
#define AS_CSTRING(value) \
    (((object_string_t *)AS_OBJECT(value))->chars)

// Typedefinition of a native function
typedef value_t (*native_function_t)(int32_t argCount, value_t *args);

// Different type of objects
typedef enum
{
    // A method the is bound to an object
    OBJECT_BOUND_METHOD,
    // A instance of a cellox class
    OBJECT_INSTANCE,
    // A class in cellox
    OBJECT_CLASS,
    // A closure
    OBJECT_CLOSURE,
    // A cellox function
    OBJECT_FUNCTION,
    // A native function
    OBJECT_NATIVE,
    // A string
    OBJECT_STRING,
    // An upvalue
    OBJECT_UPVALUE,
} object_type_t;

// struct containing the data that defines an object
struct object_t
{
    // The type of the object
    object_type_t type;
    // Determines whether the object has already been marked by the grabage collector
    bool isMarked;
    // pointer to the next object in the linear sequence of objects stored on the heap
    struct object_t * next;
};

// struct containing the data that defines a cellox function
typedef struct
{
    // data that defines all types of objects
    object_t obj;
    // The number of arguments a function expects
    uint32_t arity;
    // Number of values from enclosing scopes
    uint32_t upvalueCount;
    // The instructions in the function
    chunk_t chunk;
    // The name of the function
    object_string_t * name;
} object_function_t;

// Type definition of a native function structure
typedef struct
{
    // data that defines all types of objects
    object_t obj;
    // Reference to the native implementation in c
    native_function_t function;
} object_native_t;

// ObjectString structure definition
struct object_string_t
{
    //  data that defines all types of objects
    object_t obj;
    // The length of the string
    uint32_t length;
    // Pointer to the address in memory under that the string is stored
    char * chars;
    // The hashValue of the string
    uint32_t hash;
};

// Type definition of an object up-value structure (a local variable in an enclosing function)
typedef struct object_upvalue_t
{
    // data that defines all types of objects
    object_t obj;
    // location of the upvalue in memory
    value_t * location;
    // The Enclosed value after the current environment is left
    value_t closed;
    // The memory location of the next upvalue in memory
    struct object_upvalue_t * next;
} object_upvalue_t;

/*
 * Type definition of a closure, also called lexical closure or function closure.
 * A closure is the combination of a function and references to its surrounding state).
 * The closures in cellox are based on the closures used by the LuaVM.
 * In other words, a closure gives you access to an outer function's scope from an inner function.
 * https://en.wikipedia.org/wiki/Closure_(computer_programming)
 */
typedef struct
{
    // data that defines all types of objects
    object_t obj;
    object_function_t * function;
    object_upvalue_t ** upvalues;
    uint32_t upvalueCount;
} object_closure_t;

// Type definition of a class structure - a class in cellox
typedef struct
{
    // data that defines all types of objects
    object_t obj;
    object_string_t * name;
    table_t methods;
} object_class_t;

// Type definition of a cellox class instance
typedef struct
{
    // data that defines all types of objects
    object_t obj;
    object_class_t * celloxClass;
    table_t fields;
} object_instance_t;

// Type definition of a bound method
typedef struct
{
    // data that defines all types of objects
    object_t obj;
    value_t receiver;
    object_closure_t * method;
} object_bound_method_t;

// Copys the value of a string in the hashtable of the virtualMachine
object_string_t * object_copy_string(char const * chars, uint32_t length, bool removeBackSlash);

// Creates a new bound method
object_bound_method_t * object_new_bound_method(value_t receiver, object_closure_t * method);

// Creates a new class in cellox
object_class_t * object_new_class(object_string_t * name);

// Creates a new Closure
object_closure_t * object_new_closure(object_function_t * function);

// Creates a new cellox function
object_function_t * object_new_function();

// Creates a new cellox class instance
object_instance_t * object_new_instance(object_class_t * celloxClass);

// Creates a new native function
object_native_t * object_new_native(native_function_t function);

// Takes a string from the hashtable of the virtualMachine and returns it
object_string_t * object_take_string(char * chars, uint32_t length);

// Creates a new upvalue
object_upvalue_t * object_new_upvalue(value_t * slot);

// Prints the object
void object_print(value_t value);

// Determines whether a value is of a given type
static inline bool object_is_type(value_t value, object_type_t type)
{
    return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

#endif