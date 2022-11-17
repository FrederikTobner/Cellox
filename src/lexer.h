#ifndef CELLOX_LEXER_H_
#define CELLOX_LEXER_H_

#include "common.h"

/// @brief Tokens of the language
typedef enum
{
  /// and
  TOKEN_AND,
  /// !
  TOKEN_BANG,
  /// !=
  TOKEN_BANG_EQUAL,
  /// class
  TOKEN_CLASS,
  /// ,
  TOKEN_COMMA,
  /// .
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
  /// >
  TOKEN_GREATER,
  /// >=
  TOKEN_GREATER_EQUAL,
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
  /// -
  TOKEN_MINUS,
  /// -=
  TOKEN_MINUS_EQUAL,
  /// %
  TOKEN_MODULO,
  /// %=
  TOKEN_MODULO_EQUAL,
  /// literal nil-value/null-value/undefiened value
  TOKEN_NULL,
  /// A number literal e.g. 5.3
  TOKEN_NUMBER,
  /// or
  TOKEN_OR,
  /// print
  TOKEN_PRINT,
  /// return
  TOKEN_RETURN,
  /// }
  TOKEN_RIGHT_BRACE,
  /// )
  TOKEN_RIGHT_PAREN,
  /// ]
  TOKEN_RIGHT_BRACKET,
  /// +
  TOKEN_PLUS,
  /// +=
  TOKEN_PLUS_EQUAL,
  /// ;
  TOKEN_SEMICOLON,
  /// /
  TOKEN_SLASH,
  /// /=
  TOKEN_SLASH_EQUAL,
  /// *
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
} tokentype_t;

/// @brief A token of the cellox programming language
typedef struct
{
  /// The type of the token
  tokentype_t type;
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
