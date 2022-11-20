#include "value.h"

#include <stdio.h>

#include "object.h"

/// The value types of cellox as a string
static char const * valueTypesStringified [] = {
  "boolean",
  "undefiened",
  "numerical",
};

void value_print(value_t value)
{
#ifdef NAN_BOXING
  if (IS_BOOL(value))
    printf(AS_BOOL(value) ? "true" : "false");
  else if (IS_NULL(value))
    printf("null");
  else if (IS_NUMBER(value))
    printf("%g", AS_NUMBER(value));
  else if (IS_OBJECT(value))
    object_print(value);

#else
  switch (value.type)
  {
  case VAL_BOOL:
    printf(AS_BOOL(value) ? "true" : "false");
    break;
  case VAL_NULL:
    printf("null");
    break;
  case VAL_NUMBER:
    printf("%g", AS_NUMBER(value));
    break;
  case VAL_OBJ:
    object_print(value);
    break;
  }
#endif
}

char const * value_stringify_type(value_t value)
{
#ifdef NAN_BOXING
  if (IS_BOOL(value))
    return valueTypesStringified[0];
  else if (IS_NULL(value))
    return valueTypesStringified[1];
  else if (IS_NUMBER(value))
    return valueTypesStringified[2];
  else
  {
    return object_stringify_type(AS_OBJECT(value));
  }
#else
  switch (value.type)
  {
  case VAL_BOOL:
    return valueTypesStringified[0];
  case VAL_NULL:
    return valueTypesStringified[1];
  case VAL_NUMBER:
    return valueTypesStringified[2];
  default:
      return object_stringify_type(AS_OBJECT(value));
  }
#endif
}

bool value_values_equal(value_t a, value_t b)
{
#ifdef NAN_BOXING
  if (IS_NUMBER(a) && IS_NUMBER(b))
    return AS_NUMBER(a) == AS_NUMBER(b);
  return a == b;
#else
  if (a.type != b.type)
    return false;
  switch (a.type)
  {
  case VAL_BOOL:
    return AS_BOOL(a) == AS_BOOL(b);
  case VAL_NULL:
    return true;
  case VAL_NUMBER:
    return AS_NUMBER(a) == AS_NUMBER(b);
  case VAL_OBJ:
    return AS_OBJECT(a) == AS_OBJECT(b);
  default:
    return false; // Unreachable.
  }
#endif
}
