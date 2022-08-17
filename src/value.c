#include "value.h"

#include <stdio.h>

#include "object.h"

void printValue(Value value)
{
#ifdef NAN_BOXING
  if (IS_BOOL(value))
    printf(AS_BOOL(value) ? "true" : "false");
  else if (IS_NULL(value))
    printf("null");
  else if (IS_NUMBER(value))
    printf("%g", AS_NUMBER(value));
  else if (IS_OBJECT(value))
    printObject(value);

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
    printObject(value);
    break;
  }
#endif
}

bool valuesEqual(Value a, Value b)
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
