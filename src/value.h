#ifndef cellox_value_h
#define cellox_value_h

#include <string.h>

#include "common.h"

typedef struct object_t object_t;
typedef struct object_string_t object_string_t;

#ifdef NAN_BOXING

#define SIGN_BIT \
    ((uint64_t)0x8000000000000000)
// quiet not a number 🤫
#define QNAN \
    ((uint64_t)0x7ffc000000000000)

#define TAG_NIL 0x1
#define TAG_FALSE 0x2
#define TAG_TRUE 0x3

typedef uint64_t value_t;

// Makro that determines whether a value is of the type bool
#define IS_BOOL(value) \
    (((value) | 1) == TRUE_VAL)
// Makro that determines whether a value is nil
#define IS_NULL(value) \
    ((value) == NULL_VAL)
// Makro that determines whether a value is of the type number
#define IS_NUMBER(value) \
    (((value)&QNAN) != QNAN)
// Makro that determines whether a value is of the type obejct
#define IS_OBJECT(value) \
    (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

// Makro that yields the value of a boolean and converts to a c boolean
#define AS_BOOL(value) \
    ((value) == TRUE_VAL)
// Makro that yields the value of a number and converts to a c double
#define AS_NUMBER(value) \
    valueToNum(value)
// Makro that yields the value of an object and converts to an object pointer
#define AS_OBJECT(value) \
    ((object_t *)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))

// Makro that yields the boolean value stored in a Value
#define BOOL_VAL(b) \
    ((b) ? TRUE_VAL : FALSE_VAL)
// Makro that yields a false value
#define FALSE_VAL \
    ((value_t)(uint64_t)(QNAN | TAG_FALSE))
// Makro that yields a true value
#define TRUE_VAL \
    ((value_t)(uint64_t)(QNAN | TAG_TRUE))
// Makro that yields null
#define NULL_VAL \
    ((value_t)(uint64_t)(QNAN | TAG_NIL))
// Makro that yields the numerical value
#define NUMBER_VAL(num) \
    numToValue(num)
// Makro that yields the value of a object
#define OBJECT_VAL(obj) \
    (value_t)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

// Converts the value of a number to a Value using type punning
static inline value_t numToValue(double num)
{
    value_t value;
    memcpy(&value, &num, sizeof(double));
    return value;
}
// Converts a Value to a number using type punning
static inline double valueToNum(value_t value)
{
    double num;
    memcpy(&num, &value, sizeof(value_t));
    return num;
}

#else

// Type definition of the types of values that can be stored in a dynamic array
typedef enum
{
    // Boolean value
    VAL_BOOL,
    // Null value
    VAL_NULL,
    // A numerical value
    VAL_NUMBER,
    // A cellox object
    VAL_OBJ
} ValueType;

// Type definition of a value structure that can bew either a boolean, a number or an object (e.g. a string or a cellox object)
typedef struct
{
    ValueType type;
    union
    {
        bool boolean;
        double number;
        object_t * obj;
    } as;
} value_t;

// Makro that determines whether a value is of the type bool
#define IS_BOOL(value) \
    ((value).type == VAL_BOOL)
// Makro that determines whether a value is of the type nil
#define IS_NULL(value) \
    ((value).type == VAL_NULL)
// Makro that determines whether a value is of the type number
#define IS_NUMBER(value) \
    ((value).type == VAL_NUMBER)
// Makro that determines whether a value is of the type object
#define IS_OBJECT(value) \
    ((value).type == VAL_OBJ)

// Makro that returns the boolean value in the union
#define AS_BOOL(value) \
    ((value).as.boolean)
// Makro that returns the value of a number in the union
#define AS_NUMBER(value) \
    ((value).as.number)
// Makro that returns the value of an object in the union
#define AS_OBJECT(value) \
    ((value).as.obj)

// Makro that creates a boolean value
#define BOOL_VAL(value) \
    ((value_t){VAL_BOOL, {.boolean = value}})
// Makro that creates a null value
#define NULL_VAL \
    ((value_t){VAL_NULL, {.number = 0}})
// Makro that creates a numerical value
#define NUMBER_VAL(value) \
    ((value_t){VAL_NUMBER, {.number = value}})
// Makro that creates an object
#define OBJECT_VAL(object) \
    ((value_t){VAL_OBJ, {.obj = (object_t *)object}})
// Makro that creates a boolean value that is true
#define TRUE_VAL (BOOL_VAL(true))
// Makro that creates a boolean value that is false
#define FALSE_VAL (BOOL_VAL(false))

#endif

// prints a value
void value_print(value_t value);

// Determines whether two values are equal
bool value_values_equal(value_t a, value_t b);

#endif