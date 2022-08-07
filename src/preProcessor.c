#include "preProcessor.h"

#include <string.h>
#include <stdio.h>

#include "common.h"

typedef struct
{
    bool readingStringLiteral;
    int32_t current;
    char *source;
} PreProcessor;

PreProcessor preProcessor;

static void advance();
static void initPreProcessor();
static char peek();
static char peekNext();
static void preProcessSource();

void preProcess(char **str)
{
    initPreProcessor(str);
    preProcessSource();
}

static void advance()
{
    preProcessor.current++;
}

static void initPreProcessor(char **str)
{
    preProcessor.readingStringLiteral = false;
    preProcessor.current = 0;
    preProcessor.source = *str;
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
        if (c == '\\' && preProcessor.readingStringLiteral)
        {

            if (peekNext() == 'a')
                preProcessor.source[preProcessor.current + 1] = '\a';
            else if (peekNext() == 'b')
                preProcessor.source[preProcessor.current + 1] = '\b';
            else if (peekNext() == 'n')
                preProcessor.source[preProcessor.current + 1] = '\n';
            else if (peekNext() == 'r')
                preProcessor.source[preProcessor.current + 1] = '\r';
            else if (peekNext() == 't')
                preProcessor.source[preProcessor.current + 1] = '\t';
            else if (peekNext() == 'v')
                preProcessor.source[preProcessor.current + 1] = '\v';
            else
            {
                printf("Unknown escape sequence");
                return;
            }
            int j;
            for (j = preProcessor.current; j < strlen(preProcessor.source); j++)
                preProcessor.source[j] = preProcessor.source[j + 1];
            preProcessor.source[j] = '\0';
        }
        else if (c == '\"')
        {
            preProcessor.readingStringLiteral = !preProcessor.readingStringLiteral;
            advance();
        }
        else if (c == '#')
        {
            // TODO handle preprocessor statements - include, define, ifndef and endif
            advance();
        }
        else
            advance();
    }
}