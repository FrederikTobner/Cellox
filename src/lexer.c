#include "lexer.h"

#include <stdio.h>
#include <string.h>

// Type definition of a lexer/scanner
typedef struct
{
    // Pointer to the start of the current line where the lexical analysis is performed
    const char * start;
    // Pointer to the current position in the current line where the lexical analysis is performed
    const char * current;
    // Line counter - used for error reporting
    uint32_t line;
} lexer_t;

// Global Lexer variable
lexer_t lexer;

static char lexer_advance();
static tokentype_t lexer_check_keyword(uint32_t , uint32_t , char const *, tokentype_t);
static token_t lexer_error_token(char const *);
static token_t lexer_identifier();
static tokentype_t lexer_identifier_type();
static bool lexer_is_alpha(char);
static bool lexer_is_digit(char);
static bool lexer_is_at_end();
static token_t lexer_make_token(tokentype_t);
static bool lexer_match(char);
static token_t lexer_number();
static char lexer_peek();
static char lexer_peek_next();
static void lexer_skip_whitespace();
static token_t string();

void lexer_init(char const * sourcecode)
{
    lexer.start = lexer.current = sourcecode;
    lexer.line = 1u;
}

token_t scan_token()
{
    lexer_skip_whitespace();
    lexer.start = lexer.current;

    if (lexer_is_at_end())
        return lexer_make_token(TOKEN_EOF);
    char c = lexer_advance();
    if (lexer_is_alpha(c))
        return lexer_identifier();
    if (lexer_is_digit(c))
        return lexer_number();

    switch (c)
    {
    case '(':
        return lexer_make_token(TOKEN_LEFT_PAREN);
    case ')':
        return lexer_make_token(TOKEN_RIGHT_PAREN);
    case '{':
        return lexer_make_token(TOKEN_LEFT_BRACE);
    case '}':
        return lexer_make_token(TOKEN_RIGHT_BRACE);
    case '[':
        return lexer_make_token(TOKEN_LEFT_BRACKET);
    case ']':
        return lexer_make_token(TOKEN_RIGHT_BRACKET);
    case ';':
        return lexer_make_token(TOKEN_SEMICOLON);
    case ',':
        return lexer_make_token(TOKEN_COMMA);
    case '.':
        return lexer_make_token(TOKEN_DOT);
    case '-':
        return lexer_make_token(lexer_match('=') ? TOKEN_MINUS_EQUAL : TOKEN_MINUS);
    case '+':
        if (lexer_match('='))
            return lexer_make_token(TOKEN_PLUS_EQUAL);
        return lexer_make_token(TOKEN_PLUS);
    case '/':
        if (lexer_match('/'))
        {
            while (!lexer_match('\n'))
                lexer_advance();
            lexer.line++;
            return scan_token();
        }
        else if (lexer_match('*'))
        {
            uint32_t commentDepth = 1;
            while (commentDepth && !lexer_is_at_end())
            {
                if (lexer_match('*') && lexer_match('/'))
                    commentDepth--;
                else if (lexer_match('/') && lexer_match('*'))
                    commentDepth++;
                else if (lexer_match('\n'))
                    lexer.line++;
                else
                    lexer_advance();
            }
            if(commentDepth && lexer_is_at_end())
                return lexer_error_token("Unterminated comment");
            return scan_token();
        }
        return lexer_make_token(
            lexer_match('=') ? TOKEN_SLASH_EQUAL : TOKEN_SLASH);
    case '*':
        if (lexer_match('*'))
            return lexer_make_token(lexer_match('=') ? TOKEN_STAR_STAR_EQUAL : TOKEN_STAR_STAR);
        else if (lexer_match('='))
            return lexer_make_token(TOKEN_STAR_EQUAL);
        else
            return lexer_make_token(TOKEN_STAR);
    case '!':
        return lexer_make_token(
            lexer_match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
        return lexer_make_token(
            lexer_match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '"':
        return string();
    case ':':
        return lexer_make_token(TOKEN_DOUBLEDOT);
    case '<':
        return lexer_make_token(lexer_match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
        return lexer_make_token(lexer_match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '%':
        return lexer_make_token(lexer_match('=') ? TOKEN_MODULO_EQUAL : TOKEN_MODULO);
    case '|':
        return lexer_match('|') ? lexer_make_token(TOKEN_OR) : lexer_error_token("There is no bitwise or in cellox");
    case '&':
        return lexer_match('&') ? lexer_make_token(TOKEN_AND) : lexer_error_token("There is no bitwise and in cellox");
    }
    return lexer_error_token("Unexpected character.");
}

// Advances a position further in the sourceCode and returns the prevoius Token
static char lexer_advance()
{
    return *lexer.current++;
}

// Checks for a reserved keyword or returns a identifier token if the word is not a reserved keyword
static tokentype_t lexer_check_keyword(uint32_t start, uint32_t length, char const * rest, tokentype_t type)
{
    if (lexer.current - lexer.start == start + length && memcmp(lexer.start + start, rest, length) == 0)
    {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

// Creates an error Token with a message
static token_t lexer_error_token(char const * message)
{
    token_t token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int32_t)strlen(message);
    token.line = lexer.line;
    return token;
}

// Creates a new identifier token
static token_t lexer_identifier()
{
    while (lexer_is_alpha(lexer_peek()) || lexer_is_digit(lexer_peek()))
    {
        lexer_advance();
    }
    return lexer_make_token(lexer_identifier_type());
}

// Creates a new identifier token or a reserved keyword
static tokentype_t lexer_identifier_type()
{
    switch (lexer.start[0])
    {
    case 'a':
        return lexer_check_keyword(1, 2, "nd", TOKEN_AND);
    case 'c':
        return lexer_check_keyword(1, 4, "lass", TOKEN_CLASS);
    case 'e':
        return lexer_check_keyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
        if (lexer.current - lexer.start > 1)
        {
            // Switch for the branches coming of the 'f' node (a -> 'false', o -> 'for' and u -> 'fun')
            switch (lexer.start[1])
            {
            case 'a':
                return lexer_check_keyword(2, 3, "lse", TOKEN_FALSE);
            case 'o':
                return lexer_check_keyword(2, 1, "r", TOKEN_FOR);
            case 'u':
                return lexer_check_keyword(2, 1, "n", TOKEN_FUN);
            }
        }
        break;
    case 'i':
        return lexer_check_keyword(1, 1, "f", TOKEN_IF);
    case 'n':
        return lexer_check_keyword(1, 3, "ull", TOKEN_NULL);
    case 'o':
        return lexer_check_keyword(1, 1, "r", TOKEN_OR);
    case 'p':
        return lexer_check_keyword(1, 4, "rint", TOKEN_PRINT);
    case 'r':
        return lexer_check_keyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':
        return lexer_check_keyword(1, 4, "uper", TOKEN_SUPER);
    case 't':
        if (lexer.current - lexer.start > 1)
        {
            // Switch for the branches coming of the t node (h -> this and r -> true)
            switch (lexer.start[1])
            {
            case 'h':
                return lexer_check_keyword(2, 2, "is", TOKEN_THIS);
            case 'r':
                return lexer_check_keyword(2, 2, "ue", TOKEN_TRUE);
            }
        }
        break;
    case 'v':
        return lexer_check_keyword(1, 2, "ar", TOKEN_VAR);
    case 'w':
        return lexer_check_keyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

/* Checks if the char c is from the alphabet or an underscore.
These are the valid characters for identifiers.*/
static bool lexer_is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// checks if the char c is a digit
static bool lexer_is_digit(char c)
{
    return c >= '0' && c <= '9';
}

// Determines wheather we reached the end in the sourceCode
static bool lexer_is_at_end()
{
    return *lexer.current == '\0';
}

// Creates a new Token of a given type
static token_t lexer_make_token(tokentype_t type)
{
    token_t token;
    token.type = type;
    token.start = lexer.start;
    token.length = (int32_t)(lexer.current - lexer.start);
    token.line = lexer.line;
    return token;
}

// Matches the character at the current position of the lexer in the sourcecode with a given character
static bool lexer_match(char expected)
{
    if (lexer_is_at_end())
        return false;
    if (*lexer.current != expected)
        return false;
    lexer.current++;
    return true;
}

// Creates a new number literal token
static token_t lexer_number()
{
    while (lexer_is_digit(lexer_peek()))
    {
        lexer_advance();
    }

    // Look for a fractional part.
    if (lexer_peek() == '.' && lexer_is_digit(lexer_peek_next()))
    {
        // Consume the ".".
        lexer_advance();
        while (lexer_is_digit(lexer_peek()))
        {
            lexer_advance();
        }
    }
    return lexer_make_token(TOKEN_NUMBER);
}

/// Returns the char at the current position
static char lexer_peek()
{
    return *lexer.current;
}

// Returns the char one position ahead of the current Position
static char lexer_peek_next()
{
    if (lexer_is_at_end())
        return '\0';
    return lexer.current[1];
}

// Skips whitespaces, linebreaks, carriage returns comments an tabstobs
static void lexer_skip_whitespace()
{
    for (;;)
    {
        char c = lexer_peek();
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            // Whitespaces tabstops and carriage returns are ignored
            lexer_advance();
            break;
        case '\n':
            // Linecounter will increase on a linefeed
            lexer.line++;
            lexer_advance();
            break;
        case '/':
            if (lexer_peek_next() == '/')
            {
                // A comment goes until the end of the line.
                while (lexer_peek() != '\n' && !lexer_is_at_end())
                {
                    lexer_advance();
                }
            }
            else
            {
                return;
            }
            break;
        default:
            return;
        }
    }
}

// Creates a new string literal token
static token_t string()
{
    while (lexer_peek() != '"' && !lexer_is_at_end())
    {
        if (lexer_peek() == '\n')
            lexer.line++;
        if (lexer_peek() == '\\')
            lexer_advance();
        lexer_advance();
    }
    if (lexer_is_at_end())
        return lexer_error_token("Unterminated string.");
    // The closing quote.
    lexer_advance();
    return lexer_make_token(TOKEN_STRING);
}
