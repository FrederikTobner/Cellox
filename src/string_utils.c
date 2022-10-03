#include "string_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void string_utils_behead(char *, int *);

// Checks if a string contains the specified character restricted the specified length
bool string_utils_contains_character_restricted(char const * text, char character, uint32_t length)
{
    for (uint32_t i = 0u; i < length; i++)
    {
        if (!*(text + i))
            return false;
        if (*(text + i) == character)
            return true;
    }
    return false;
}

// Resolves all the escape sequences inside a string literal
int string_utils_resolve_escape_sequence(char * text, uint32_t * length)
{
    string_utils_behead(text, length);
    switch (*text)
    {
    // escape sequence using a number in octal format
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
        {
            uint32_t octalValue = 0u;
            uint8_t octalLoopCounter = 1u;
            do
            {
                octalValue *= 8u;
                octalValue += *text -'0';
                if(!(*(text + 1) >= '0' && *(text + 1) <= '7'))
                    break; // No octal number ahead
                string_utils_behead(text, length);
            } while (octalLoopCounter++ < 3);
            if(octalValue > 255)
                return -1;
            if(octalLoopCounter == 4)
                return -1;
            *text = (char)octalValue;
            break;
        }
    // escape sequence using a number in hexadezimal format
    case 'x':
        {
            uint32_t hexValue = 0u;
            uint8_t hexLoopCounter = 1u;
            string_utils_behead(text, length);
            if(!(*text >= '0' && *text <= '9') && !(*(text + 1) >= 'a' && *(text + 1) <= 'f'))
                return -1;
            do
            {
                hexValue *= 16u;            
                hexValue += *text > '9' ? *text - 'a' + 10 : *text -'0';
                if(!(*(text + 1) >= '0' && *(text + 1) <= '9') && !(*(text + 1)>= 'a' && *(text + 1) <= 'f'))
                    break; // No hexadezimal number ahead
                string_utils_behead(text, length);
            } while (hexLoopCounter++ < 2);
            if(hexLoopCounter == 3)
                return -1;
            *text = (char)hexValue;
            break;
        }
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
        return -1;
    }
    return 0;    
}

// Removes the first character in a sequence of characters
static void string_utils_behead(char * text, int * length)
{
    uint32_t stringLength = strlen(text);
    for (uint32_t j = 0u; j < stringLength; j++)
        *(text + j) = *(text + j + 1);
    *(text + stringLength) = '\0';
    (*length)--;
}