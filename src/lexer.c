#include "lexer.h"

#include <stdio.h>
#include <string.h>

// Type definition of a lexer/lexer stucture
typedef struct
{
    // Pointer to the start of the current line where the lexical analysis is performed
    const char *start;
    // Pointer to the current position in the current line where the lexical analysis is performed
    const char *current;
    // Line counter - used for error reporting
    int32_t line;
} Lexer;

// Global Lexer variable
Lexer lexer;

static char advance();
static TokenType checkKeyword(int32_t start, int32_t length, const char *rest, TokenType type);
static Token errorToken(const char *message);
static Token identifier();
static TokenType identifierType();
static bool isAlpha(char c);
static bool isDigit(char c);
static bool isAtEnd();
static Token makeToken(TokenType type);
static bool match(char expected);
static Token number();
static char peek();
static char peekNext();
static void skipWhitespace();
static Token string();

void initLexer(const char *source)
{
    lexer.start = source;
    lexer.current = source;
    lexer.line = 1;
}

Token scanToken()
{
    skipWhitespace();
    lexer.start = lexer.current;

    if (isAtEnd())
        return makeToken(TOKEN_EOF);
    char c = advance();
    if (isAlpha(c))
        return identifier();
    if (isDigit(c))
        return number();

    switch (c)
    {
    case '(':
        return makeToken(TOKEN_LEFT_PAREN);
    case ')':
        return makeToken(TOKEN_RIGHT_PAREN);
    case '{':
        return makeToken(TOKEN_LEFT_BRACE);
    case '}':
        return makeToken(TOKEN_RIGHT_BRACE);
    case ';':
        return makeToken(TOKEN_SEMICOLON);
    case ',':
        return makeToken(TOKEN_COMMA);
    case '.':
        return makeToken(TOKEN_DOT);
    case '-':
        return makeToken(match('=') ? TOKEN_MINUS_EQUAL : TOKEN_MINUS);
    case '+':
        if (match('='))
            return makeToken(TOKEN_PLUS_EQUAL);
        return makeToken(TOKEN_PLUS);
    case '/':
        if (match('/'))
        {
            while (!match('\n'))
                advance();
            lexer.line++;
            return scanToken();
        }
        else if (match('*'))
        {
            uint32_t commentDepth = 1;
            while (commentDepth)
            {
                if (match('*') && match('/'))
                    commentDepth--;
                else if (match('/') && match('*'))
                    commentDepth++;
                else if (match('\n'))
                    lexer.line++;
                else
                    advance();
            }
            return scanToken();
        }
        return makeToken(
            match('=') ? TOKEN_SLASH_EQUAL : TOKEN_SLASH);
    case '*':
        if (match('*'))
            return makeToken(match('=') ? TOKEN_STAR_STAR_EQUAL : TOKEN_STAR_STAR);
        else if (match('='))
            return makeToken(TOKEN_STAR_EQUAL);
        else
            return makeToken(TOKEN_STAR);
    case '!':
        return makeToken(
            match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
        return makeToken(
            match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '"':
        return string();
    case ':':
        return makeToken(TOKEN_DOUBLEDOT);
    case '<':
        return makeToken(
            match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
        return makeToken(
            match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '%':
        return makeToken(
            match('=') ? TOKEN_MODULO_EQUAL : TOKEN_MODULO);
    case '|':
        return match('|') ? makeToken(TOKEN_OR) : errorToken("There is no bitwise or in cellox");
    case '&':
        return match('&') ? makeToken(TOKEN_AND) : errorToken("There is no bitwise and in cellox");
    }
    return errorToken("Unexpected character.");
}

// Advances a position further in the sourceCode and returns the prevoius Token
static char advance()
{
    lexer.current++;
    return lexer.current[-1];
}

// Checks for a reserved keyword or returns a identifier token if the word is not a reserved keyword
static TokenType checkKeyword(int32_t start, int32_t length, const char *rest, TokenType type)
{
    if (lexer.current - lexer.start == start + length &&
        memcmp(lexer.start + start, rest, length) == 0)
    {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

// Creates an error Token with a message
static Token errorToken(const char *message)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int32_t)strlen(message);
    token.line = lexer.line;
    return token;
}

// Creates a new identifier token
static Token identifier()
{
    while (isAlpha(peek()) || isDigit(peek()))
    {
        advance();
    }
    return makeToken(identifierType());
}

// Creates a new identifier token or a reserved keyword
static TokenType identifierType()
{
    switch (lexer.start[0])
    {
    case 'a':
        return checkKeyword(1, 2, "nd", TOKEN_AND);
    case 'c':
        return checkKeyword(1, 4, "lass", TOKEN_CLASS);
    case 'e':
        return checkKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
        if (lexer.current - lexer.start > 1)
        {
            // Switch for the branches coming of the 'f' node (a -> 'false', o -> 'for' and u -> 'fun')
            switch (lexer.start[1])
            {
            case 'a':
                return checkKeyword(2, 3, "lse", TOKEN_FALSE);
            case 'o':
                return checkKeyword(2, 1, "r", TOKEN_FOR);
            case 'u':
                return checkKeyword(2, 1, "n", TOKEN_FUN);
            }
        }
        break;
    case 'i':
        return checkKeyword(1, 1, "f", TOKEN_IF);
    case 'n':
        return checkKeyword(1, 3, "ull", TOKEN_NULL);
    case 'o':
        return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p':
        return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r':
        return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':
        return checkKeyword(1, 4, "uper", TOKEN_SUPER);
    case 't':
        if (lexer.current - lexer.start > 1)
        {
            // Switch for the branches coming of the t node (h -> this and r -> true)
            switch (lexer.start[1])
            {
            case 'h':
                return checkKeyword(2, 2, "is", TOKEN_THIS);
            case 'r':
                return checkKeyword(2, 2, "ue", TOKEN_TRUE);
            }
        }
        break;
    case 'v':
        return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w':
        return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

/* Checks if the char c is from the alphabet or an underscore.
These are the valid characters for identifiers.*/
static bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// checks if the char c is a digit
static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

// Determines wheather we reached the end in the sourceCode
static bool isAtEnd()
{
    return *lexer.current == '\0';
}

// Creates a new Token of a given type
static Token makeToken(TokenType type)
{
    Token token;
    token.type = type;
    token.start = lexer.start;
    token.length = (int32_t)(lexer.current - lexer.start);
    token.line = lexer.line;
    return token;
}

// Matches the character at the current position of the lexer in the sourcecode with a given character
static bool match(char expected)
{
    if (isAtEnd())
        return false;
    if (*lexer.current != expected)
        return false;
    lexer.current++;
    return true;
}

// Creates a new number literal token
static Token number()
{
    while (isDigit(peek()))
    {
        advance();
    }

    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext()))
    {
        // Consume the ".".
        advance();
        while (isDigit(peek()))
        {
            advance();
        }
    }
    return makeToken(TOKEN_NUMBER);
}

/// Returns the char at the current position
static char peek()
{
    return *lexer.current;
}

// Returns the char one position ahead of the current Position
static char peekNext()
{
    if (isAtEnd())
        return '\0';
    return lexer.current[1];
}

// Skips whitespaces, linebreaks, carriage returns comments an tabstobs
static void skipWhitespace()
{
    for (;;)
    {
        char c = peek();
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            // Whitespaces tabstops and carriage returns are ignored
            advance();
            break;
        case '\n':
            // Linecounter will increase on a linefeed
            lexer.line++;
            advance();
            break;
        case '/':
            if (peekNext() == '/')
            {
                // A comment goes until the end of the line.
                while (peek() != '\n' && !isAtEnd())
                {
                    advance();
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
static Token string()
{
    while (peek() != '"' && !isAtEnd())
    {
        if (peek() == '\n')
            lexer.line++;
        if (peek() == '\\')
            advance();
        advance();
    }

    if (isAtEnd())
    {
        return errorToken("Unterminated string.");
    }

    // The closing quote.
    advance();
    return makeToken(TOKEN_STRING);
}