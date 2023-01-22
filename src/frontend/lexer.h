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
 * @file lexer.h
 * @brief Header file containing of declarations regarding the lexer.
 * @details A lexer, also called scanner, coverts a sequence of characters into a sequence of tokens.
 */

#ifndef CELLOX_LEXER_H_
#define CELLOX_LEXER_H_

#include "../common.h"

/// @brief Tokens of the language
typedef enum
{
  /// and / &amp;%amp;
  TOKEN_AND,
  /// !
  TOKEN_BANG,
  /// !=
  TOKEN_BANG_EQUAL,
  /// A number literal specified in binary
  TOKEN_BINARY_NUMBER,
  /// class
  TOKEN_CLASS,
  /// ,
  TOKEN_COMMA,
  /// do
  TOKEN_DO,
  /// &sdot;
  TOKEN_DOT,
  /// : (Used for inheritance)
  TOKEN_DOUBLEDOT,
  /// else
  TOKEN_ELSE,
  /// Used to mark the end of the file
  TOKEN_EOF,
  /// =
  TOKEN_EQUAL,
  /// ==
  TOKEN_EQUAL_EQUAL,
  /// Used to mark an error in the parsing process
  TOKEN_ERROR,
  /// Boolean literal false
  TOKEN_FALSE,
  /// for
  TOKEN_FOR,
  /// fun
  TOKEN_FUN,
  /// &gt;
  TOKEN_GREATER,
  /// >=
  TOKEN_GREATER_EQUAL,
  /// A number literal specified in hex
  TOKEN_HEX_NUMBER,
  /// An identifier e.g. for the variable named x
  TOKEN_IDENTIFIER,
  /// if
  TOKEN_IF,
  /// {
  TOKEN_LEFT_BRACE,
  /// (
  TOKEN_LEFT_PAREN,
  /// [
  TOKEN_LEFT_BRACKET,
  /// <
  TOKEN_LESS,
  /// <=
  TOKEN_LESS_EQUAL,
  /// &minus;
  TOKEN_MINUS,
  /// -=
  TOKEN_MINUS_EQUAL,
  /// %
  TOKEN_MODULO,
  /// %=
  TOKEN_MODULO_EQUAL,
  /// literal null-value&frasl;undefiened value
  TOKEN_NULL,
  /// A number literal e.g. 5.3
  TOKEN_NUMBER,
  /// or
  TOKEN_OR,
  /// print
  TOKEN_PRINT,
  /// println
  TOKEN_PRINT_LINE,
  /// &sdot;&sdot;
  TOKEN_RANGE,
  /// return
  TOKEN_RETURN,
  /// }
  TOKEN_RIGHT_BRACE,
  /// )
  TOKEN_RIGHT_PAREN,
  /// ]
  TOKEN_RIGHT_BRACKET,
  /// \+
  TOKEN_PLUS,
  /// +=
  TOKEN_PLUS_EQUAL,
  /// ;
  TOKEN_SEMICOLON,
  /// &frasl;
  TOKEN_SLASH,
  /// /=
  TOKEN_SLASH_EQUAL,
  /// \*;
  TOKEN_STAR,
  /// *=
  TOKEN_STAR_EQUAL,
  /// **
  TOKEN_STAR_STAR,
  /// **=
  TOKEN_STAR_STAR_EQUAL,
  /// A string literal e.g. "Hello World!"
  TOKEN_STRING,
  /// super
  TOKEN_SUPER,
  /// this
  TOKEN_THIS,
  /// Boolean literal true
  TOKEN_TRUE,
  /// var
  TOKEN_VAR,
  /// while
  TOKEN_WHILE,
} tokentype;

/// @brief A token of the cellox programming language
typedef struct
{
  /// The type of the token
  tokentype type;
  /// Pointer to the start of the character sequence
  char const * start;
  /// Length of the character sequence
  uint32_t length;
  /// Line in the sourceCode
  uint32_t line;
} token_t;

/// @brief Initializes the lexer
/// @param sourcecode The sourcecode that is used for initialitizing the lexer
void lexer_init(char const * sourcecode);

/// @brief Scans the next token in the sourcecode and saves it in a linear sequence of tokens
/// @return The Next Token in the sourcecode
token_t scan_token();

#endif
