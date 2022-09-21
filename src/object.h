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
    ((ObjectBoundMethod *)AS_OBJECT(value))
// Makro that gets the value of an object as a cellox class instance
#define AS_INSTANCE(value) \
    ((ObjectInstance *)AS_OBJECT(value))
// Makro that gets the value of an object as a class
#define AS_CLASS(value) \
    ((ObjectClass *)AS_OBJECT(value))
// Makro that gets the value of an object as a closure
#define AS_CLOSURE(value) \
    ((ObjectClosure *)AS_OBJECT(value))
// Makro that gets the value of an object as a function
#define AS_FUNCTION(value) \
    ((ObjectFunction *)AS_OBJECT(value))
// Makro that gets the value of an object as a native function
#define AS_NATIVE(value) \
    (((ObjectNative *)AS_OBJECT(value))->function)
// Makro that gets the value of an object as a string
#define AS_STRING(value) \
    ((ObjectString *)AS_OBJECT(value))
// Makro that gets the value of an object as a cstring
#define AS_CSTRING(value) \
    (((ObjectString *)AS_OBJECT(value))->chars)

// Typedefinition of a native function
typedef Value (*NativeFn)(int32_t argCount, Value *args);

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
} ObjectType;

// struct containing the data that defines an object
struct Object
{
    // The type of the object
    ObjectType type;
    // Determines whether the object has already been marked by the grabage collector
    bool isMarked;
    // pointer to the next object in the linear sequence of objects stored on the heap
    struct Object *next;
};

// struct containing the data that defines a cellox function
typedef struct
{
    // data that defines all types of objects
    Object obj;
    // The number of arguments a function expects
    uint32_t arity;
    // Number of values from enclosing scopes
    uint32_t upvalueCount;
    // The instructions in the function
    Chunk chunk;
    // The name of the function
    ObjectString *name;
} ObjectFunction;

// Type definition of a native function structure
typedef struct
{
    // data that defines all types of objects
    Object obj;
    // Reference to the native implementation in c
    NativeFn function;
} ObjectNative;

// ObjectString structure definition
struct ObjectString
{
    //  data that defines all types of objects
    Object obj;
    // The length of the string
    uint32_t length;
    // Pointer to the address in memory under that the string is stored
    char *chars;
    // The hashValue of the string
    uint32_t hash;
};

// Type definition of an object up-value structure (a local variable in an enclosing function)
typedef struct ObjectUpvalue
{
    // data that defines all types of objects
    Object obj;
    // location of the upvalue in memory
    Value *location;
    // The Enclosed value after the current environment is left
    Value closed;
    // The memory location of the next upvalue in memory
    struct ObjectUpvalue *next;
} ObjectUpvalue;

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
    Object obj;
    ObjectFunction *function;
    ObjectUpvalue **upvalues;
    uint32_t upvalueCount;
} ObjectClosure;

// Type definition of a class structure - a class in cellox
typedef struct
{
    // data that defines all types of objects
    Object obj;
    ObjectString *name;
    Table methods;
} ObjectClass;

// Type definition of a cellox class instance
typedef struct
{
    // data that defines all types of objects
    Object obj;
    ObjectClass *celloxClass;
    Table fields;
} ObjectInstance;

// Type definition of a bound method
typedef struct
{
    // data that defines all types of objects
    Object obj;
    Value receiver;
    ObjectClosure *method;
} ObjectBoundMethod;

// Copys the value of a string in the hashtable of the virtualMachine
ObjectString *object_copy_string(char const *chars, uint32_t length, bool removeBackSlash);

// Creates a new bound method
ObjectBoundMethod *object_new_bound_method(Value receiver, ObjectClosure *method);

// Creates a new class in cellox
ObjectClass *object_new_class(ObjectString *name);

// Creates a new Closure
ObjectClosure *object_new_closure(ObjectFunction *function);

// Creates a new cellox function
ObjectFunction *object_new_function();

// Creates a new cellox class instance
ObjectInstance *object_new_instance(ObjectClass *celloxClass);

// Creates a new native function
ObjectNative *object_new_native(NativeFn function);

// Takes a string from the hashtable of the virtualMachine and returns it
ObjectString *object_take_string(char *chars, uint32_t length);

// Creates a new upvalue
ObjectUpvalue *object_new_upvalue(Value *slot);

// Prints the object
void object_print(Value value);

// Determines whether a value is of a given type
static inline bool object_is_type(Value value, ObjectType type)
{
    return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

#endif