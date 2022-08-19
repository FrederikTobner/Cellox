#include "preProcessor.h"

#include <string.h>
#include <stdio.h>

#include "common.h"

// Contains data relevant for the pre-processor
typedef struct
{
    // Pointer to the start of the current line where the preprocessing is performed
    const char *start;
    // Pointer to the current position in the current line where the preprocessing is performed
    const char *current;
} PreProcessor;

// Global preProcessor variable
PreProcessor preProcessor;

static char advance();
static void initPreProcessor();
static bool isAtEnd();
static void preProcessSource();

void preProcess(char *str)
{
    initPreProcessor(str);
    preProcessSource();
}

static char advance()
{
    return *preProcessor.current++;
}

static void initPreProcessor(char *str)
{
    preProcessor.start = str;
    preProcessor.current = preProcessor.start;
}

static bool isAtEnd()
{
    return preProcessor.current == '\0';
}

static void preProcessSource()
{
    char c;
    while (!isAtEnd())
    {
        c = advance();
        if (c == '#')
        {
            advance();
        }
    }
}