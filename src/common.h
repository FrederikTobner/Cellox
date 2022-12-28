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
 * @file common.h
 * @brief Common header file of the interpreter
 * @details Contains commnly used includes and exit codes
 */

#ifndef CELLOX_COMMON_H_
#define CELLOX_COMMON_H_

/// @brief Exit codes of the cellox interpreter
/// @details These exit codes are based on the exit codes defined in the UNIX sysexits.h header file.
/// They can be found under at "/usr/include/sysexits.h"
typedef enum
{
    /// Signals a Sucess
    EXIT_CODE_OK = 0,
    /// Signals a command line error usage
    EXIT_CODE_COMMAND_LINE_USAGE_ERROR = 64,
    /// Signals an error that occured during compile time
    EXIT_CODE_COMPILATION_ERROR = 65,
    /// Signals an error that occured during runtime time
    EXIT_CODE_RUNTIME_ERROR = 70,
    /// Signals an error that occured during system time
    EXIT_CODE_SYSTEM_ERROR = 71,
    /// Signals an error regarding the input or the output
    EXIT_CODE_INPUT_OUTPUT_ERROR = 74
}interpreter_exit_code_t;

// Bool from the C99 standard
#include <stdbool.h>

// Provides a null pointer constant
#include <stddef.h>

// Typedefs for exact-width-integer types from the C99 standard
#include <stdint.h>

/// Number of values in a dynamic array of upvalues or locals
#define UINT8_COUNT (256)

#endif // CELLOX_COMMON_H_
