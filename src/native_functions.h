#ifndef CELLOX_NATIVE_FUNCTIONS_H_
#define CELLOX_NATIVE_FUNCTIONS_H_

#include "common.h"
#include "value.h"

// Configuration of a native function
typedef struct
{
char const * functionName;
void * function;
size_t arrity;
}native_function_config_t;


// Returns an array that contains the native function configurations
native_function_config_t * native_get_function_configs();

// The amount of native functions that are defiened
size_t native_get_function_count();

// Determines the class of a cellox instance
value_t native_classof(uint32_t argCount, value_t const * args);

// Native clock function - used for benchmarks
value_t native_clock(uint32_t argCount, value_t const * args);

// Native Exit function
value_t native_exit(uint32_t argCount, value_t const * args);

// Gets the user name
value_t native_get_username(uint32_t argCount, value_t const * args);

// Native random function - returns a random number
value_t native_random(uint32_t argCount, value_t const * args);

// Native readLine function
value_t native_read_line(uint32_t argCount, value_t const * args);

// Returns true if the program is executed on a linux system and false if not
value_t native_on_linux(uint32_t argCount, value_t const * args);

// Returns true if the program is executed on a windows system and false if not
value_t native_on_windows(uint32_t argCount, value_t const * args);

// Determines the length of a string
value_t native_string_length(uint32_t argCount, value_t const * args);

// Used to execute a terminal command
value_t native_system(uint32_t argCount, value_t const * args);

// Native wait function
value_t native_wait(uint32_t argCount, value_t const * args);

#endif