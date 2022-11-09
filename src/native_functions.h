#ifndef CELLOX_NATIVE_FUNCTIONS_H_
#define CELLOX_NATIVE_FUNCTIONS_H_

#include "common.h"
#include "value.h"

/// @brief Configuration of a native function
typedef struct
{
char const * functionName;
void * function;
size_t arrity;
}native_function_config_t;

/// @brief Typedefinition of a native function
typedef value_t (*native_function_t)(int32_t argCount, value_t *args);

/// @brief Gets the configuration of the native functions
/// @return an array that contains the native function configurations 
native_function_config_t * native_functions_get_function_configs();

/// @brief Gets the amount of defined native functions
/// @return The amount of native functions that are defiened 
size_t native_functions_get_function_count();

/// @brief Determines the class of a cellox instance
/// @param argCount The amount of arguments that were used when class_of was called
/// @param args The arguments that class_of was called with
/// @return The class of the instance
value_t native_functions_classof(uint32_t argCount, value_t const * args);

/// @brief Native clock function - used for benchmarks
/// @param argCount The amount of arguments that were used when clock was called
/// @param args The arguments that clock was called with
/// @return The amount of seconds that have passed since the program execution started
value_t native_functions_clock(uint32_t argCount, value_t const * args);

/// @brief Native Exit function
/// @param argCount The amount of arguments that were used when exit was called
/// @param args The arguments that exit was called with
/// @return Nothing because the program will exit
value_t native_functions_exit(uint32_t argCount, value_t const * args);

/// @brief Gets the user name
/// @param argCount The amount of arguments that were used when get_username was called
/// @param args The aguments that get_username was called with
/// @return An object string that represents the user name
value_t native_functions_get_username(uint32_t argCount, value_t const * args);

/// @brief Native random function
/// @param argCount The amount of arguments that were used when random was called
/// @param args The arguments that random was called with
/// @return a random number
value_t native_functions_random(uint32_t argCount, value_t const * args);

/// @brief Reads the next character from the standard input
/// @param argCount The amount of arguments that were used when read_key was called
/// @param args The aguments that read_key was called with
/// @return The key that was read
value_t native_functions_read_key(uint32_t argCount, value_t const * args);

/// @brief Reads the next line of characters from the standard input
/// @param argCount The amount of arguments that were used when read_line was called
/// @param args The aguments that read_line was called with
/// @return The line that was read
value_t native_functions_read_line(uint32_t argCount, value_t const * args);

/// @brief Determines whether the program is executed under linux
/// @param argCount The amount of arguments that were used when on_linux was called
/// @param args The aguments that on_linux was called with
/// @return true if the program is executed on a linux system and false if not
value_t native_functions_on_linux(uint32_t argCount, value_t const * args);

/// @brief Determines whether the program is executed under windows
/// @param argCount The amount of arguments that were used when on_windows was called
/// @param args The aguments that on_windows was called with
/// @return true if the program is executed on a windows system and false if not
value_t native_functions_on_windows(uint32_t argCount, value_t const * args);

/// @brief Determines the length of a string
/// @param argCount The amount of arguments that were used when strlen was called
/// @param args The aguments that strlen was called with
/// @return The length of the string
value_t native_functions_string_length(uint32_t argCount, value_t const * args);

/// @brief  Used to execute a system call
/// @param argCount The amount of arguments that were used when system was called
/// @param args The aguments that system was called with
/// @return NULL
value_t native_functions_system(uint32_t argCount, value_t const * args);

/// @brief Native wait function - waits for the specified amount of seconds
/// @param argCount The amount of arguments that were used when wait was called
/// @param args The aguments that wait was called with
/// @return NULL
value_t native_functions_wait(uint32_t argCount, value_t const * args);

#endif