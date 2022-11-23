#ifndef CELLOX_CHUNK_FILE_H_
#define CELLOX_CHUNK_FILE_H_

#include <stdbool.h>

#include "chunk.h"

/// @brief Compiler flags
typedef enum
{
/// Compile flag that indicates that line information is included in the chunk file
COMPILE_FLAG_LINE_INFO_INCLUDED     =       0b00000001u,
/// Compile flag that indicates that line information is included in the chunk file
COMPILE_FLAG_ANONYMIZE_FUNCTIONS    =       0b00000010u,
/// Compile flag that indicates whether the contents have already been optimized in the chunk file (may be removed)
COMPILE_FLAG_OPTIMIZE               =       0b00000100u,
/// All compile flags combined
COMPILE_FLAG_ALL                    =       0b11111111u
}chunk_file_compile_flags;

/**
 * @brief Stores the chunks that were generated by compiling a cellox program as a file
 * @details A cellox bytecode file has the following structure
 * ----file metadata------
 * ----top level chunk----
 * @param chunk The chunk that is stored
 * @param programmPath The program path of the cellox program
 * @param flags Compile flags used to compile the sourcecode to a cellox chunk file
 * @return 0 -> OK, -1 -> Error
 */
int chunk_file_store(chunk_t chunk, char const * programmPath, chunk_file_compile_flags flags);

/// @brief Creates a chunk based on a cellox bytecode file
/// @param filePath The path of the cellox bytecode file
/// @return Pointer to the created chunk
chunk_t * chunk_file_load(char const * filePath);

#endif
