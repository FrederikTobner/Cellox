#ifndef clox_object_h
#define clox_object_h

#include "chunk.h"
#include "common.h"
#include "value.h"

// Makro that determines the type of an object
#define OBJ_TYPE(value) (AS_OBJ(value)->type)

// Makro that determines if the object has the object type closure
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
// Makro that determines if the object has the object type function
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
// Makro that determines if the object has the object type native - native function
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
// Makro that determines if the object has the object type string
#define IS_STRING(value) isObjType(value, OBJ_STRING)

// Makro that gets the value of an object as a closure
#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))
// Makro that gets the value of an object as a function
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
// Makro that gets the value of an object as a native function
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)
// Makro that gets the value of an object as a string
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
// Makro that gets the value of an object as a cstring
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)

// Different type of objects
typedef enum
{
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
    int arity;
    // Number of values from enclosing scopes
    int upvalueCount;
    // The instructions in the function
    Chunk chunk;
    // The name of the function
    ObjString *name;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value *args);

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
    int length;
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
    int upvalueCount;
} ObjClosure;

// Creates a new Closure
ObjClosure *newClosure(ObjFunction *function);
// Creates a new kellox function
ObjFunction *newFunction();
// Creates a new native function
ObjNative *newNative(NativeFn function);
// Deletes a string frm the hashtable of the vm and returns it
ObjString *takeString(char *chars, int length);
// Copys the value of a string in the hashtable of the vm
ObjString *copyString(const char *chars, int length);
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