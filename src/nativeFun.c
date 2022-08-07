#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "nativeFun.h"
#include "object.h"
#include "value.h"

Value clockNative(int32_t argCount, Value *args)
{
    if (argCount > 0)
    {
        printf("clock has an arity of 0");
        return NUMBER_VAL(-1.0);
    }
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

Value randomNative(int32_t argCount, Value *args)
{
    if (argCount > 0)
    {
        printf("random has an arity of 0");
        return NUMBER_VAL(-1.0);
    }
    return NUMBER_VAL(rand());
}
