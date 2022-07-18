#ifndef clox_value_h
#define clox_value_h

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

// Type definition of the types of values that can be stored in a dynamic array
typedef enum
{
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ
} ValueType;

// Type definition of a value structure that can bew either a boolean, a number or an object (e.g. a string or a kellox object)
typedef struct
{
    ValueType type;
    union
    {
        bool boolean;
        double number;
        Obj *obj;
    } as;
} Value;

// Makro that determines whether a value is of the type bool
#define IS_BOOL(value) ((value).type == VAL_BOOL)
// Makro that determines whether a value is of the type nil
#define IS_NIL(value) ((value).type == VAL_NIL)
// Makro that determines whether a value is of the type number
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
// Makro that determines whether a value is of the type object
#define IS_OBJ(value) ((value).type == VAL_OBJ)

// Makro that returns the boolean value in the union
#define AS_BOOL(value) ((value).as.boolean)
// Makro that returns the value of a number in the union
#define AS_NUMBER(value) ((value).as.number)
// Makro that returns the value of an object in the union
#define AS_OBJ(value) ((value).as.obj)

// Makro that creates a boolean value
#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
// Makro that creates a nil value
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
// Makro that creates a numerical value
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
// Makro that creates an object
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj *)object}})

// Type definition of the dynamic array
typedef struct
{
    int capacity;
    int count;
    Value *values;
} ValueArray;

// Determines whether two values are equal
bool valuesEqual(Value a, Value b);

// initializes the dynamic array
void initValueArray(ValueArray *array);

// Adds a value to the dynamic array
void writeValueArray(ValueArray *array, Value value);

// Dealocates the memory used by the dynamic array
void freeValueArray(ValueArray *array);

// prints a value
void printValue(Value value);

#endif