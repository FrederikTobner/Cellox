#ifndef clox_value_h
#define clox_value_h

#include <string.h>

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

#ifdef NAN_BOXING

#define SIGN_BIT ((uint64_t)0x8000000000000000)
#define QNAN ((uint64_t)0x7ffc000000000000)

#define TAG_NIL 1   // 01.
#define TAG_FALSE 2 // 10.
#define TAG_TRUE 3  // 11.

typedef uint64_t Value;

#define IS_BOOL(value) \
    (((value) | 1) == TRUE_VAL)
#define IS_NIL(value) \
    ((value) == NIL_VAL)
#define IS_NUMBER(value) \
    (((value)&QNAN) != QNAN)
#define IS_OBJ(value) \
    (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define AS_BOOL(value) \
    ((value) == TRUE_VAL)
#define AS_NUMBER(value) \
    valueToNum(value)
#define AS_OBJ(value) \
    ((Obj *)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))

#define BOOL_VAL(b) \
    ((b) ? TRUE_VAL : FALSE_VAL)
#define FALSE_VAL \
    ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL \
    ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define NIL_VAL \
    ((Value)(uint64_t)(QNAN | TAG_NIL))
#define NUMBER_VAL(num) \
    numToValue(num)
#define OBJ_VAL(obj) \
    (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

static inline double valueToNum(Value value)
{
    double num;
    memcpy(&num, &value, sizeof(Value));
    return num;
}

static inline Value numToValue(double num)
{
    Value value;
    memcpy(&value, &num, sizeof(double));
    return value;
}

#else

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

#endif

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

// Type definition of the dynamic value array
typedef struct
{
    int32_t capacity;
    int32_t count;
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