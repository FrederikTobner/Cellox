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

// Type definition of a value that can be stored in the dynamic array
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

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value) ((value).as.obj)

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
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