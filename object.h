#ifndef clox_object_h
#define clox_object_h

#include "chunk.h"
#include "common.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)

// Differnt type of objects
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
    ObjType type;
    bool isMarked;
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
    int length;
    char *chars;
    uint32_t hash;
};

// Type definition of an object up-value structure (a local variable in an enclosing function)
typedef struct ObjUpvalue
{
    Obj obj;
    Value *location;
    Value closed;
    struct ObjUpvalue *next;
} ObjUpvalue;

/*Type definition of a closure.
 *A closure is the combination of a function and references to its surrounding state).
 * In other words, a closure gives you access to an outer function's scope from an inner function
 https://en.wikipedia.org/wiki/Closure_(computer_programming)*/
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