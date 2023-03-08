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
 * @file lexer.c
 * @brief File containing implementation of the lexer.
 * @details A lexer, also called scanner, coverts a sequence of characters into a sequence of tokens, in a process
 * called lexical analysis.
 */

#include "lexer.h"

#include <stdio.h>
#include <string.h>

/// @brief A lexer or more commonly called scanner
/// @details It is used to scan the tokens in a cellox source file
typedef struct {
    /// Pointer to the start of the current token where the lexical analysis is performed
    char const * start;
    /// Pointer to the current position in the current token where the lexical analysis is performed
    char const * current;
    /// Line counter - used for error reporting
    uint32_t line;
} lexer_t;

/// Global Lexer variable
lexer_t lexer;

static inline char lexer_advance();
static tokentype lexer_check_keyword(uint32_t, uint32_t, char const *, tokentype);
static token_t lexer_error_token(char const *);
static token_t lexer_identifier();
static tokentype lexer_identifier_type();
static inline bool lexer_is_alpha(char);
static inline bool lexer_is_digit(char);
static inline bool lexer_is_at_end();
static token_t lexer_make_token(tokentype);
static bool lexer_match(char);
static token_t lexer_number();
static inline char lexer_peek();
static char lexer_peek_next();
static void lexer_skip_whitespace();
static token_t lexer_string();

void lexer_init(char const * sourcecode) {
    lexer.start = lexer.current = sourcecode;
    lexer.line = 1u;
}

token_t lexer_scan_token() {
    #define MAKE_TOKEN_CASE(character, token) \
        case character:\
        return lexer_make_token(token)
    
    lexer_skip_whitespace();
    lexer.start = lexer.current;

    if (lexer_is_at_end()) {
        return lexer_make_token(TOKEN_EOF);
    }
    char c = lexer_advance();
    if (lexer_is_alpha(c)) {
        return lexer_identifier();
    }
    if (lexer_is_digit(c)) {
        return lexer_number();
    }

    switch (c) {
        MAKE_TOKEN_CASE('(', TOKEN_LEFT_PAREN);
        MAKE_TOKEN_CASE(')', TOKEN_RIGHT_PAREN);
        MAKE_TOKEN_CASE('{', TOKEN_LEFT_BRACE);
        MAKE_TOKEN_CASE('}', TOKEN_RIGHT_BRACE);
        MAKE_TOKEN_CASE('[', TOKEN_LEFT_BRACKET);
        MAKE_TOKEN_CASE(']', TOKEN_RIGHT_BRACKET);
        MAKE_TOKEN_CASE(';', TOKEN_SEMICOLON);
        MAKE_TOKEN_CASE(',', TOKEN_COMMA);
        MAKE_TOKEN_CASE('.', lexer_match('.') ? TOKEN_RANGE : TOKEN_DOT);
        MAKE_TOKEN_CASE('-', lexer_match('=') ? TOKEN_MINUS_EQUAL : TOKEN_MINUS);
        MAKE_TOKEN_CASE('+', lexer_match('=') ? TOKEN_PLUS_EQUAL : TOKEN_PLUS);
    case '/':
        if (lexer_match('/')) {
            while (!lexer_match('\n')) {
                lexer_advance();
            }
            lexer.line++;
            return lexer_scan_token();
        } else if (lexer_match('*')) {
            uint32_t commentDepth = 1;
            while (commentDepth && !lexer_is_at_end()) {
                if (lexer_match('*') && lexer_match('/')) {
                    commentDepth--;
                } else if (lexer_match('/') && lexer_match('*')) {
                    commentDepth++;
                } else if (lexer_match('\n')) {
                    lexer.line++;
                } else {
                    lexer_advance();
                }
            }
            if (commentDepth && lexer_is_at_end()) {
                return lexer_error_token("Unterminated comment");
            }
            return lexer_scan_token();
        }
        return lexer_make_token(lexer_match('=') ? TOKEN_SLASH_EQUAL : TOKEN_SLASH);
    case '*':
        if (lexer_match('*')) {
            return lexer_make_token(lexer_match('=') ? TOKEN_STAR_STAR_EQUAL : TOKEN_STAR_STAR);
        } else if (lexer_match('=')) {
            return lexer_make_token(TOKEN_STAR_EQUAL);
        } else {
            return lexer_make_token(TOKEN_STAR);
        }
    MAKE_TOKEN_CASE('!', lexer_match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    MAKE_TOKEN_CASE('=', lexer_match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '"':
        return lexer_string();
    MAKE_TOKEN_CASE(':', TOKEN_DOUBLEDOT);
    MAKE_TOKEN_CASE('<', lexer_match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    MAKE_TOKEN_CASE('>', lexer_match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    MAKE_TOKEN_CASE('%', lexer_match('=') ? TOKEN_MODULO_EQUAL : TOKEN_MODULO);
    case '|':
        return lexer_match('|') ? lexer_make_token(TOKEN_OR) : lexer_error_token("There is no bitwise or in cellox");
    case '&':
        return lexer_match('&') ? lexer_make_token(TOKEN_AND) : lexer_error_token("There is no bitwise and in cellox");
    }
    return lexer_error_token("Unexpected character.");
    #undef MAKE_TOKEN_CASE
}

/// Advances a position further in the sourceCode and returns the prevoius Token
static inline char lexer_advance() {
    return *lexer.current++;
}

/// Checks for a reserved keyword or returns a identifier token if the word is not a reserved keyword
static tokentype lexer_check_keyword(uint32_t start, uint32_t length, char const * rest, tokentype type) {
    if (lexer.current - lexer.start == start + length && !memcmp(lexer.start + start, rest, length)) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

/// Creates an error Token with a message
static token_t lexer_error_token(char const * message) {
    token_t token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int32_t)strlen(message);
    token.line = lexer.line;
    return token;
}

/// Creates a new identifier token
static token_t lexer_identifier() {
    while (lexer_is_alpha(lexer_peek()) || lexer_is_digit(lexer_peek())) {
        lexer_advance();
    }
    return lexer_make_token(lexer_identifier_type());
}

/// Creates a new identifier token or a reserved keyword
static tokentype lexer_identifier_type() {
    #define CASE_KEYWORD(character, start, length, rest, token) \
        case character: \
        return lexer_check_keyword(start, length, rest, token);
    switch (lexer.start[0]) {
    CASE_KEYWORD('a', 1, 2, "nd", TOKEN_AND);
    CASE_KEYWORD('c', 1, 4, "lass", TOKEN_CLASS);
    CASE_KEYWORD('d', 1, 1, "o", TOKEN_DO);
    CASE_KEYWORD('e', 1, 3, "lse", TOKEN_ELSE);
    case 'f':
        if (lexer.current - lexer.start > 1) {
            switch (lexer.start[1]) {
            case 'a':
                return lexer_check_keyword(2, 3, "lse", TOKEN_FALSE);
            case 'o':
                return lexer_check_keyword(2, 1, "r", TOKEN_FOR);
            case 'u':
                return lexer_check_keyword(2, 1, "n", TOKEN_FUN);
            }
        }
        break;
    CASE_KEYWORD('i', 1, 1, "f", TOKEN_IF);
    CASE_KEYWORD('n', 1, 3, "ull", TOKEN_NULL);
    CASE_KEYWORD('o', 1, 1, "r", TOKEN_OR);
    CASE_KEYWORD('r', 1, 5, "eturn", TOKEN_RETURN);
    CASE_KEYWORD('s', 1, 4, "uper", TOKEN_SUPER);
    case 't':
        if (lexer.current - lexer.start > 1) {
            switch (lexer.start[1]) {
            case 'h':
                return lexer_check_keyword(2, 2, "is", TOKEN_THIS);
            case 'r':
                return lexer_check_keyword(2, 2, "ue", TOKEN_TRUE);
            }
        }
        break;
    CASE_KEYWORD('v', 1, 2, "ar", TOKEN_VAR);
    CASE_KEYWORD('w', 1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
    #undef CASE_KEYWORD
}

/// @brief Checks if the char c is from the alphabet or an underscore.
/// @details These are the valid characters for identifiers.
static inline bool lexer_is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

/// @brief checks if a character is a digit (0-9)
/// @param c The character that is checked
/// @return True if the character is a digit, false if not
static inline bool lexer_is_digit(char c) {
    return c >= '0' && c <= '9';
}

/// @brief Determines wheather we reached the end in the sourcecode
/// @return True if we reached the end, false if not
static inline bool lexer_is_at_end() {
    return *lexer.current == '\0';
}

/// @brief Creates a new Token of a given type
/// @param type The type of the type that is generated
/// @return The Token that was created
static token_t lexer_make_token(tokentype type) {
    token_t token;
    token.type = type;
    token.start = lexer.start;
    token.length = (int32_t)(lexer.current - lexer.start);
    token.line = lexer.line;
    return token;
}

/// Matches the character at the current position of the lexer in the sourcecode with a given character
static bool lexer_match(char expected) {
    if (lexer_is_at_end()) {
        return false;
    }
    if (*lexer.current != expected) {
        return false;
    }
    lexer.current++;
    return true;
}

/// Creates a new number literal token
static token_t lexer_number() {
    if (lexer_peek() == '0') {
        lexer_advance();
    }
    if (lexer_peek() == 'x' || lexer_peek() == 'X') {
        // Hexadezimal number
        lexer_advance();
        char c = lexer_peek();
        size_t length = 0;
        while (lexer_is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
            lexer_advance();
            c = lexer_peek();
            length++;
        }
        if (length > 8) {
            return lexer_error_token("Hexadezimal number literals have a maximum length of 8 digits");
        }
        return lexer_make_token(TOKEN_HEX_NUMBER);
    } else if (lexer_peek() == 'b' || lexer_peek() == 'B') {
        // Binary number
        lexer_advance();
        char c = lexer_peek();
        size_t length = 0;
        while (lexer_peek() == '0' || lexer_peek() == '1') {
            lexer_advance();
            c = lexer_peek();
            length++;
        }
        if (length > 32) {
            return lexer_error_token("Hexadezimal number literals have a maximum length of 32 digits");
        }
        return lexer_make_token(TOKEN_BINARY_NUMBER);
    }

    while (lexer_is_digit(lexer_peek())) {
        lexer_advance();
    }

    // Look for a fractional part.
    if (lexer_peek() == '.' && lexer_is_digit(lexer_peek_next())) {
        // Consume the ".".
        lexer_advance();
        while (lexer_is_digit(lexer_peek())) {
            lexer_advance();
        }
    }
    return lexer_make_token(TOKEN_NUMBER);
}

/// Returns the char at the current position
static inline char lexer_peek() {
    return *lexer.current;
}

/// Returns the char one position ahead of the current Position
static char lexer_peek_next() {
    if (lexer_is_at_end()) {
        return '\0';
    }
    return lexer.current[1];
}

/// Skips whitespaces, linebreaks, carriage returns comments an tabstobs
static void lexer_skip_whitespace() {
    for (;;) {
        char c = lexer_peek();
        switch (c) {
        case ' ':
        case '\r':
        case '\t':
            // Whitespaces, tabstops, and carriage returns are ignored
            lexer_advance();
            break;
        case '\n':
            lexer.line++;
            lexer_advance();
            break;
        case '/':
            if (lexer_peek_next() == '/') {
                while (lexer_peek() != '\n' && !lexer_is_at_end()) { // A comment goes until the end of the line.
                    lexer_advance();
                }
            } else {
                return;
            }
            break;
        default:
            return;
        }
    }
}

/// @brief Creates a new string literal token
/// @return The string literal token that was created
static token_t lexer_string() {
    while (lexer_peek() != '"' && !lexer_is_at_end()) {
        if (lexer_peek() == '\n') {
            lexer.line++;
        }
        if (lexer_peek() == '\\') {
            lexer_advance();
        }
        lexer_advance();
    }
    if (lexer_is_at_end()) {
        return lexer_error_token("Unterminated string.");
    }
    // Skips the closing quote.
    lexer_advance();
    return lexer_make_token(TOKEN_STRING);
}
