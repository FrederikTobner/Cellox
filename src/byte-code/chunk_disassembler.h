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
 * @file debug.h
 * @brief File containing declarations of functions that are used to debug the interpreter.
 */

#ifndef CELLOX_DEBUG_H_
#define CELLOX_DEBUG_H_

#include "../byte-code/chunk.h"

/// @brief  Dissasembles a chunk of bytecode instructions
/// @param chunk The chunk that is dissasembled
/// @param name The name of the chunk (based on the function)
/// @param arity The arity of the top level funtion of the chunk
/// @details First prints the metadata of the chunk (function, string constant, numerical constant count).
/// Then prints all the opcodes that are stored in the chunk 
void chunk_disassembler_disassemble_chunk(chunk_t * chunk, char const * name, uint32_t arity);

/// @brief Dissasembles a single instruction
/// @param chunk The chunk where a single instruction is dissasembled
/// @param offset The offset of the instruction
/// @return The offset of the next instruction
/// @details Prints the opcode and the correspoonding line number
int32_t chunk_disassembler_disassemble_instruction(chunk_t * chunk, int32_t offset);

#endif
