#include "preProcessor.h"

#include <string.h>
#include <stdio.h>

#include "common.h"

// Contains data relevant for the pre-processor
typedef struct
{
    int32_t current;
    char *source;
} PreProcessor;

// Global preProcessor variable
PreProcessor preProcessor;

static void advance();
static void initPreProcessor();
static char peek();
static char peekNext();
static void preProcessSource();

void preProcess(char *str)
{
    initPreProcessor(str);
    preProcessSource();
}

static void advance()
{
    preProcessor.current++;
}

static void initPreProcessor(char *str)
{
    preProcessor.current = 0;
    preProcessor.source = str;
}

static char peek()
{
    return preProcessor.source[preProcessor.current];
}

static char peekNext()
{
    return preProcessor.source[preProcessor.current + 1];
}

static void preProcessSource()
{
    char c;
    while (preProcessor.current < strlen(preProcessor.source))
    {
        c = peek();
        if (c == '#')
        {
            // TODO handle preprocessor statements - include, define, ifndef and endif
            advance();
        }
        else
            advance();
    }
}