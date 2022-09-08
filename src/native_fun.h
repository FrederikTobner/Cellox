#ifndef cellox_native_fun_h
#define cellox_native_fun_h

#include "common.h"
#include "value.h"

// Native clock function - used for benchmarks
Value native_clock(uint32_t argCount, Value *args);

// Native Exit function
Value native_exit(uint32_t argCount, Value *args);

// Gets the user name
Value native_get_username(uint32_t argCount, Value *args);

// Native random function - returns a random number
Value native_random(uint32_t argCount, Value *args);

// Native readLine function
Value native_read_line(uint32_t argCount, Value *args);

// Native wait function
Value native_wait(uint32_t argCount, Value *args);

#endif