#ifndef cellox_nativeFun_h
#define cellox_nativeFun_h

#include "common.h"
#include "value.h"

// Native clock function - used for benchmarks
Value clockNative(int32_t argCount, Value *args);
// Native Exit function
Value exitNative(int32_t argCount, Value *args);
// Gets the user name
Value getUserNameNative(int32_t argCount, Value *args);
// Native random function - returns a random number
Value randomNative(int32_t argCount, Value *args);
// Native readLine function
Value readLineNative(int32_t argCount, Value *args);
// Native wait function
Value waitNative(int32_t argCount, Value *args);

#endif