#ifndef clox_common_h
#define clox_common_h

// C99 bool
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Flag for printing out the bytecode, can be removed if you want to disable it
//#define DEBUG_PRINT_CODE

/*Optional stress mode for the garbage collector.
* When this flag is defiened the Garbage collector runs as often as it cans.
Only used for debugging*/
//#define DEBUG_STRESS_GC

// Optional Flag for logging the garbage collection process.
//#define DEBUG_LOG_GC

// Flag for tracing the execution, can be removed if you want to disable it
//#define DEBUG_TRACE_EXECUTION

// Number of possible values for a byte
#define UINT8_COUNT (UINT8_MAX + 1)

#endif