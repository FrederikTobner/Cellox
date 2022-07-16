#ifndef clox_common_h
#define clox_common_h

// C99 bool
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Flag for printing out the bytecode, can be removed if you want to disable it
#define DEBUG_PRINT_CODE

// Flag for tracing the execution, can be removed if you want to disable it
#define DEBUG_TRACE_EXECUTION

// Flag for debugging the program
#define DEBUG_WORKAROUND

#define UINT8_COUNT (UINT8_MAX + 1)

#endif