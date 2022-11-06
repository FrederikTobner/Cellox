#include "value.h"

#include <stdio.h>

#include "object.h"

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
    return "boolean";
  else if (IS_NULL(value))
    return "undefiened";
  else if (IS_NUMBER(value))
    return "numerical";
  else
  {
    if(IS_BOUND_METHOD(value))
      return "method";
    if(IS_CLASS(value))
      return "class";
    if(IS_FUNCTION(value))
      return "function";
    if(IS_CLOSURE(value))
      return "closure";
    if(IS_INSTANCE(value))
      return AS_INSTANCE(value)->celloxClass->name->chars;
    if(IS_NATIVE(value))
      return "native function";
    if(IS_STRING(value))
      return "string";
    return "unknown";
  }
#else
  switch (value.type)
  {
  case VAL_BOOL:
    return "boolean";
  case VAL_NULL:
    return "null";
  case VAL_NUMBER:
    return "number";
  default:
    {
      if(IS_BOUND_METHOD(value))
        return "method";
      if(IS_CLASS(value))
        return "class";
      if(IS_FUNCTION(value))
        return "function";
      if(IS_CLOSURE(value))
        return "closure";
      if(IS_INSTANCE(value))
        return AS_INSTANCE(value)->celloxClass->name->chars;
      if(IS_NATIVE(value))
        return "native function";
      if(IS_STRING(value))
        return "string";
      return "unknown";
    }
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
