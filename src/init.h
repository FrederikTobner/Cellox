#ifndef CELLOX_INIT_H_
#define CELLOX_INIT_H_

// Init.h is included in the test-suite that is written in c++ using the google-test framework
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/// Message that explains the usage of the cellox interpreter
#define CELLOX_USAGE_MESSAGE ("Usage: Cellox ((-h|--help|-v|--version) | ((-scf | --store-chunk-file) |(-rcf |--read-chunk-file)) [path]\n")

/** @brief Run with repl
* @details  
* <ol>
* <li> Read the user input   -   (R)</li>
* <li> Evaluate your code    -   (E)</li>
* <li> Print any results     -   (P)</li>
* <li> Loop back to step 1   -   (L)</li>
* </ol>
*/
void init_repl();

/// @brief Reads a lox program from a file and executes the program
/// @param path The path of the lox program
/// @param compile boolean value that determines whether the program is compiled and stored as a chunk file
void init_run_from_file(char const *, bool);

/// Shows the help of the cellox interpreter
void init_show_help();

/// Shows the version of the cellox interpreter
void init_show_version();

#ifdef __cplusplus
}
#endif

#endif
