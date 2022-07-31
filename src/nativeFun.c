#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "nativeFun.h"
#include "object.h"
#include "value.h"

// Native clock function - used for benchmarks
Value clockNative(int32_t argCount, Value *args)
{
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

Value randomNative(int32_t argCount, Value *args)
{
    return NUMBER_VAL(rand());
}
