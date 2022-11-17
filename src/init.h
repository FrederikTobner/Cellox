#ifndef CELLOX_INIT_H_
#define CELLOX_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define CELLOX_USAGE_MESSAGE ("Usage: Cellox ((-h|--help|-v|--version) | ((-scf | --store-chunk-file) |(-rcf |--read-chunk-file)) [path]\n")

/** @brief Run with repl
* @details  1. Read the user input
*           2. Evaluate your code
*           3. Print any results
*           4. Loop back to step 1
*/
void init_repl();

/// @brief Reads a lox program from a file and executes the program
/// @param path The path of the lox program
/// @param compile boolean value that determines whether the program is compiled and stored as a chunk file
void init_run_from_file(char const *, bool);

/// @brief Shows the help of the cellox interpreter
void init_show_help();

/// @brief Shows the version of the cellox interpreter
void init_show_version();

#ifdef __cplusplus
}
#endif

#endif
