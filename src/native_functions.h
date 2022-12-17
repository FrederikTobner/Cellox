/****************************************************************************
 * Copyright (C) 2022 by Frederik Tobner                                    *
 *                                                                          *
 * This file is part of Cellox.                                             *
 *                                                                          *
 * Permission to use, copy, modify, and distribute this software and its    *
 * documentation under the terms of the GNU General Public License is       *
 * hereby granted.                                                          *
 * No representations are made about the suitability of this software for   *
 * any purpose.                                                             *
 * It is provided "as is" without express or implied warranty.              *
 * See the <https://www.gnu.org/licenses/gpl-3.0.html/>GNU General Public   *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file native_functions.h
 * @brief Header file containing the declarations of functionality used by the native functions of the interpreter.
 */

#ifndef CELLOX_NATIVE_FUNCTIONS_H_
#define CELLOX_NATIVE_FUNCTIONS_H_

#include "common.h"
#include "value.h"

/// Typedefinition of a native function
typedef value_t (*native_function_t)(uint32_t argCount, value_t const * args);

/// Configuration of a native function
typedef struct
{
/// The name of the native function
char const * functionName;
/// Pointer to the native function
native_function_t function;
/// The amount of arguments the native function expects
size_t arrity;
}native_function_config_t;

/// @brief Gets the configuration of the native functions
/// @return an array that contains the native function configurations 
native_function_config_t * native_functions_get_function_configs();

/// @brief Gets the amount of defined native functions
/// @return The amount of native functions that are defiened
size_t native_functions_get_function_count();

/// @brief Determines the length of an array
/// @param argCount The amount of arguments that were used when array_length was called 
/// @param args The arguments that array_length was called with
/// @return The length of the array
value_t native_functions_array_length(uint32_t argCount, value_t const * args);

/// @brief Appends the content of a cellox string to a file
/// @param argCount The amount of arguments that were used when append_to was called 
/// @param args The arguments that append_to_file was called with
/// @return True -> Sucess, False -> Error
value_t native_functions_append_to_file(uint32_t argCount, value_t const * args);

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

/// @brief Determines whether the program is executed under linux
/// @param argCount The amount of arguments that were used when on_linux was called
/// @param args The arguments that on_linux was called with
/// @return true if the program is executed on a linux system and false if not
value_t native_functions_on_linux(uint32_t argCount, value_t const * args);

/// @brief Determines whether the program is executed under macOS
/// @param argCount The amount of arguments that were used when on_macOS was called
/// @param args The aguments that on_macOS was called with
/// @return true if the program is executed on a macOS system and false if not
value_t native_functions_on_macOS(uint32_t argCount, value_t const * args);

/// @brief Determines whether the program is executed under windows
/// @param argCount The amount of arguments that were used when on_windows was called
/// @param args The arguments that on_windows was called with
/// @return true if the program is executed on a windows system and false if not
value_t native_functions_on_windows(uint32_t argCount, value_t const * args);

/// @brief Prints the value that is passed as a argument
/// @param argCount The amount of arguments that were used when print was called
/// @param args The aguments that print was called with
/// @return NULL
value_t native_functions_print(uint32_t argCount, value_t const * args);

/// @brief Prints the value that is passed as a argument formated
/// @param argCount The amount of arguments that were used when print was called
/// @param args The aguments that print was called with
/// @return NULL
value_t native_functions_print_formated(uint32_t argCount, value_t const * args);

/// @brief Prints the value that is passed as a argument and ads a automatic line break
/// @param argCount The amount of arguments that were used when println was called
/// @param args The arguments that print was called with
/// @return NULL
value_t native_functions_print_line(uint32_t argCount, value_t const * args);

/// @brief Native random function
/// @param argCount The amount of arguments that were used when random was called
/// @param args The arguments that random was called with
/// @return a random number
value_t native_functions_random(uint32_t argCount, value_t const * args);

/// @brief Reads the contents of a file and creates a cellox string based on the content of the file
/// @param argCount The amount of arguments that were used when read_file was called
/// @param args The arguments that read_file was called with
/// @return NULL if an error occured, otherwise a string that represents the content of the file
value_t native_functions_read_file(uint32_t argCount, value_t const * args);

/// @brief Reads the next character from the standard input
/// @param argCount The amount of arguments that were used when read_key was called
/// @param args The arguments that read_key was called with
/// @return The key that was read
value_t native_functions_read_key(uint32_t argCount, value_t const * args);

/// @brief Reads the next line of characters from the standard input
/// @param argCount The amount of arguments that were used when read_line was called
/// @param args The arguments that read_line was called with
/// @return The line that was read
value_t native_functions_read_line(uint32_t argCount, value_t const * args);

/// @brief Determines the size of a value
/// @param argCount The amount of arguments that were used when size_of was called
/// @param args The arguments that size_of was called with
/// @return The size of the value
value_t native_functions_size_of(uint32_t argCount, value_t const * args);

/// @brief 
/// @param argCount 
/// @param args 
/// @return 
value_t native_functions_string_hash(uint32_t argCount, value_t const * args);

/// @brief Determines the length of a string
/// @param argCount The amount of arguments that were used when strlen was called
/// @param args The arguments that strlen was called with
/// @return The length of the string
value_t native_functions_string_length(uint32_t argCount, value_t const * args);

/// @brief Replaces a character in the at the specified index
/// @param argCount The amount of arguments that were used when string_replace_at was called
/// @param args The arguments that  string_replace_at was called with
/// @return The newly created string where a single character is altered
value_t native_functions_string_replace_at(uint32_t argCount, value_t const * args);

/// @brief  Used to execute a system call
/// @param argCount The amount of arguments that were used when system was called
/// @param args The aguments that system was called with
/// @return NULL
value_t native_functions_system(uint32_t argCount, value_t const * args);

/// @brief Native wait function - waits for the specified amount of seconds
/// @param argCount The amount of arguments that were used when wait was called
/// @param args The arguments that wait was called with
/// @return NULL
value_t native_functions_wait(uint32_t argCount, value_t const * args);

/// @brief Writes the content of a cellox string to a file
/// @param argCount The amount of arguments that were used when write_to_file was called 
/// @param args The arguments that write_to_file was called with
/// @return True -> Sucess, False -> Error
value_t native_functions_write_to_file(uint32_t argCount, value_t const * args);

#endif
