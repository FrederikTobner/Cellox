#include "string_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void string_utils_behead(char *text);

// Checks if a string contains the specified character restricted the specified length
bool string_utils_contains_character_restricted(char const *text, char character, int length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        if (*(text + i) == '\0')
            return false;
        if (*(text + i) == character)
            return true;
    }
    return false;
}

// Resolves all the escape sequences inside a string literal
void string_utils_resolve_escape_sequence(char *text, int *length)
{
    switch (*(++text))
    {
    // Alarm or beep
    case 'a':
        *text = '\a';
        break;
    // Backspace
    case 'b':
        *text = '\b';
        break;
    // formfeed page break
    case 'f':
        *text = '\f';
        break;
    // New Line
    case 'n':
        *text = '\n';
        break;
    // Carriage Return
    case 'r':
        *text = '\r';
        break;
    // Tab Horizontal
    case 't':
        *text = '\t';
        break;
    // Tab vertical
    case 'v':
        *text = '\v';
        break;
    // Backslash, single and double quote
    case '\"':
        break;
    case '\'':
        break;
    case '\\':
        break;
    case '?':
        *text = '\?';
        break;
    default:
        printf("Unknown escape sequence \\%c", *text);
        exit(65);
    }
    string_utils_behead(text - 1);
    (*length)--;
}

// Removes the first character in a sequence of characters
static void string_utils_behead(char *text)
{
    // Removes '\\' that precedes the escape sequence
    for (uint32_t j = 0; j < strlen(text); j++)
        text[j] = text[j + 1];
    text = '\0';
}