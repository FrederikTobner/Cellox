#ifndef cellox_nativeFun_h
#define cellox_nativeFun_h

#include "common.h"
#include "value.h"

// Native clock function - used for benchmarks
Value clockNative(int32_t argCount, Value *args);
// Native random function - returns a random number
Value randomNative(int32_t argCount, Value *args);

#endif