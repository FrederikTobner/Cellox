#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

// Type definition of a scanner/lexer
typedef struct
{
    const char *start;
    const char *current;
    int line;
} Scanner;

// Global Scanner variable
Scanner scanner;

// Initializes the scanner
void initScanner(const char *source)
{
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

// Checks if the char c is from the alphabet
static bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

// checks if the char c is a digit
static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

// Determines wheather we reached the end in the sourceCode
static bool isAtEnd()
{
    return *scanner.current == '\0';
}

// Advances a position further in the sourceCode and returns the prevoius Token
static char advance()
{
    scanner.current++;
    return scanner.current[-1];
}

/// Returns the char at the current position
static char peek()
{
    return *scanner.current;
}

// Returns the char one position ahead of the current Position
static char peekNext()
{
    if (isAtEnd())
        return '\0';
    return scanner.current[1];
}

// Matches the character at the current position of the scanner in the sourcecode with a given character
static bool match(char expected)
{
    if (isAtEnd())
        return false;
    if (*scanner.current != expected)
        return false;
    scanner.current++;
    return true;
}

// Creates a new Token of a given type
static Token makeToken(TokenType type)
{
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

// Creates an error Token with a message
static Token errorToken(const char *message)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
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
            scanner.line++;
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

// Checks for a reserved keyword or returns a identifier token if the word is not a reserved keyword
static TokenType checkKeyword(int start, int length, const char *rest, TokenType type)
{
    if (scanner.current - scanner.start == start + length &&
        memcmp(scanner.start + start, rest, length) == 0)
    {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

// Creates a new identifier token or a reserved keyword
static TokenType identifierType()
{
    switch (scanner.start[0])
    {
    case 'a':
        return checkKeyword(1, 2, "nd", TOKEN_AND);
    case 'c':
        return checkKeyword(1, 4, "lass", TOKEN_CLASS);
    case 'e':
        return checkKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
        if (scanner.current - scanner.start > 1)
        {
            // Switch for the branches coming of the 'f' node (a -> 'false', o -> 'for' and u -> 'fun')
            switch (scanner.start[1])
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
        return checkKeyword(1, 2, "il", TOKEN_NIL);
    case 'o':
        return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p':
        return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r':
        return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':
    case 't':
        if (scanner.current - scanner.start > 1)
        {
            // Switch for the branches coming of the t node (h -> this and r -> true)
            switch (scanner.start[1])
            {
            case 'h':
                return checkKeyword(2, 2, "is", TOKEN_THIS);
            case 'r':
                return checkKeyword(2, 2, "ue", TOKEN_TRUE);
            }
        }
        break;
        return checkKeyword(1, 4, "uper", TOKEN_SUPER);
    case 'v':
        return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w':
        return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
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

// Creates a new string literal token
static Token string()
{
    while (peek() != '"' && !isAtEnd())
    {
        if (peek() == '\n')
            scanner.line++;
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

Token scanToken()
{
    skipWhitespace();
    scanner.start = scanner.current;

    if (isAtEnd())
    {
        return makeToken(TOKEN_EOF);
    }
    char c = advance();
    if (isAlpha(c))
    {
        return identifier();
    }
    if (isDigit(c))
    {
        return number();
    }

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
        return makeToken(TOKEN_MINUS);
    case '+':
        return makeToken(TOKEN_PLUS);
    case '/':
        return makeToken(TOKEN_SLASH);
    case '*':
        return makeToken(TOKEN_STAR);
    case '!':
        return makeToken(
            match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
        return makeToken(
            match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '"':
        return string();
    case '<':
        return makeToken(
            match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
        return makeToken(
            match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    }
    return errorToken("Unexpected character.");
}