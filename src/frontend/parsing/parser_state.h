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
 * See the <"https://www.gnu.org/licenses/gpl-3.0.html">GNU General Public  *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file parser_state.h
 * @brief Parser state declarations used by compiler modules.
 */

#ifndef CELLOX_PARSER_STATE_H_
#define CELLOX_PARSER_STATE_H_

#include "frontend/lexical_analysis/lexer.h"

/// @brief The cellox parser
/// @details The parser builds an abstract syntax tree out of the tokens that were produced by the lexer
typedef struct {
    /// The token that is currently being parsed
    token_t current;
    /// The token that was previously parsed
    token_t previous;
    /// Flag that indicates whether an error occured during the compilation
    bool hadError;
    /// Flag that indicates that the compiler couldn't synchronize after an errror occured
    bool panicMode;
} parser_t;

#endif