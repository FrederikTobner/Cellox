#ifndef clox_object_h
#define clox_object_h

#include "chunk.h"
#include "table.h"
#include "common.h"
#include "value.h"

// Makro that determines the type of an object
#define OBJ_TYPE(value) \
    (AS_OBJ(value)->type)

//  Makro that determines if the object has the object type bound-method
#define IS_BOUND_METHOD(value) \
    isObjType(value, OBJ_BOUND_METHOD)
//  Makro that determines if the object has the object type instance
#define IS_INSTANCE(value) \
    isObjType(value, OBJ_INSTANCE)
// Makro that determines if the object has the object type class
#define IS_CLASS(value) \
    isObjType(value, OBJ_CLASS)
// Makro that determines if the object has the object type closure
#define IS_CLOSURE(value) \
    isObjType(value, OBJ_CLOSURE)
// Makro that determines if the object has the object type function
#define IS_FUNCTION(value) \
    isObjType(value, OBJ_FUNCTION)
// Makro that determines if the object has the object type native - native function
#define IS_NATIVE(value) \
    isObjType(value, OBJ_NATIVE)
// Makro that determines if the object has the object type string
#define IS_STRING(value) \
    isObjType(value, OBJ_STRING)

//// Makro that gets the value of an object as a bound method
#define AS_BOUND_METHOD(value) \
    ((ObjBoundMethod *)AS_OBJ(value))
// Makro that gets the value of an object as a kellox class instance
#define AS_INSTANCE(value) \
    ((ObjInstance *)AS_OBJ(value))
// Makro that gets the value of an object as a class
#define AS_CLASS(value) \
    ((ObjClass *)AS_OBJ(value))
// Makro that gets the value of an object as a closure
#define AS_CLOSURE(value) \
    ((ObjClosure *)AS_OBJ(value))
// Makro that gets the value of an object as a function
#define AS_FUNCTION(value) \
    ((ObjFunction *)AS_OBJ(value))
// Makro that gets the value of an object as a native function
#define AS_NATIVE(value) \
    (((ObjNative *)AS_OBJ(value))->function)
// Makro that gets the value of an object as a string
#define AS_STRING(value) \
    ((ObjString *)AS_OBJ(value))
// Makro that gets the value of an object as a cstring
#define AS_CSTRING(value) \
    (((ObjString *)AS_OBJ(value))->chars)

// Different type of objects
typedef enum
{
    OBJ_BOUND_METHOD,
    // A instance of a kellox class
    OBJ_INSTANCE,
    // A class in kellox
    OBJ_CLASS,
    // A closure
    OBJ_CLOSURE,
    // A kellox function
    OBJ_FUNCTION,
    // A native function
    OBJ_NATIVE,
    // A string
    OBJ_STRING,
    // An upvalue
    OBJ_UPVALUE,
} ObjType;

// struct containing the data that defines an object
struct Obj
{
    // The type of the object
    ObjType type;
    // Determines whether the object has already been marked by the grabage collector
    bool isMarked;
    // pointer to the next object in the linear sequence of objects stored on the heap
    struct Obj *next;
};

// struct containing the data that defines a kellox function
typedef struct
{
    Obj obj;
    // The number of arguments a function expects
    int32_t arity;
    // Number of values from enclosing scopes
    int32_t upvalueCount;
    // The instructions in the function
    Chunk chunk;
    // The name of the function
    ObjString *name;
} ObjFunction;

typedef Value (*NativeFn)(int32_t argCount, Value *args);

// Type definition of a native function structure
typedef struct
{
    Obj obj;
    NativeFn function;
} ObjNative;

// ObjString structure definition
struct ObjString
{
    Obj obj;
    // The length of the string
    int32_t length;
    // Pointer to the address in memory under that the string is stored
    char *chars;
    // The hashValue of the string
    uint32_t hash;
};

// Type definition of an object up-value structure (a local variable in an enclosing function)
typedef struct ObjUpvalue
{
    Obj obj;
    // location of the upvalue in memory
    Value *location;
    // The Enclosed value after the current environment is left
    Value closed;
    // The memory location of the next upvalue in memory
    struct ObjUpvalue *next;
} ObjUpvalue;

/*
 * Type definition of a closure, also called lexical closure or function closure.
 * A closure is the combination of a function and references to its surrounding state).
 * The closures in kellox are based on the closures used by the LuaVM.
 * In other words, a closure gives you access to an outer function's scope from an inner function.
 * https://en.wikipedia.org/wiki/Closure_(computer_programming)
 */
typedef struct
{
    Obj obj;
    ObjFunction *function;
    ObjUpvalue **upvalues;
    int32_t upvalueCount;
} ObjClosure;

// Type definition of a class structure - a class in kellox
typedef struct
{
    Obj obj;
    ObjString *name;
    Table methods;
} ObjClass;

// Type definition of a kellox class instance
typedef struct
{
    Obj obj;
    ObjClass *kelloxClass;
    Table fields;
} ObjInstance;

// Type definition of a bound method
typedef struct
{
    Obj obj;
    Value receiver;
    ObjClosure *method;
} ObjBoundMethod;

// Copys the value of a string in the hashtable of the vm
ObjString *copyString(const char *chars, int32_t length);
// Creates a new bound method
ObjBoundMethod *newBoundMethod(Value receiver, ObjClosure *method);
// Creates a new class in kellox
ObjClass *newClass(ObjString *name);
// Creates a new Closure
ObjClosure *newClosure(ObjFunction *function);
// Creates a new kellox function
ObjFunction *newFunction();
// Creates a new kellox class instance
ObjInstance *newInstance(ObjClass *kelloxClass);
// Creates a new native function
ObjNative *newNative(NativeFn function);
// Deletes a string frm the hashtable of the vm and returns it
ObjString *takeString(char *chars, int32_t length);

// Creates a new upvalue
ObjUpvalue *newUpvalue(Value *slot);
// Prints the object
void printObject(Value value);

// Determines whether a value is of a given type
static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif