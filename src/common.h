#ifndef clox_common_h
#define clox_common_h

// Bool from the C99 standard
#include <stdbool.h>
// Provides a null pointer constant
#include <stddef.h>
// Typedefs for exact-width-integer types from the C99 standard
#include <stdint.h>

// Number of values in a dynamic array (used for upvalues and locals)
#define UINT8_COUNT (1 << 8)

// Flag for enabeling NAN boxing / NAN tagging
#define NAN_BOXING

// Flag for printing out the bytecode, can be removed if you want to disable it
//#define DEBUG_PRINT_CODE

/*Optional stress mode for the garbage collector.
* When this flag is defiened the Garbage collector runs as often as it cans.
Should only be used for debugging*/
//#define DEBUG_STRESS_GC

// Optional Flag for logging the garbage collection process.
//#define DEBUG_LOG_GC

// Flag for tracing the execution, can be removed if you want to disable it
//#define DEBUG_TRACE_EXECUTION

#endif