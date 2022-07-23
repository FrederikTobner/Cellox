#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

// Maximum length of a line is 1024 characters
#define MAX_LINE_LENGTH 1024

// Reads a lox program from a file
static char *readFile(const char *path);

/* Run with repl
 * 1. Read the user input
 * 2. Evaluate your code
 * 3. Print any results
 * 4. Loop back to step 1
 */
static void repl();

// Reads a lox program from a file and executes the program
static void runFile(const char *path);

int main(int argc, const char *argv[])
{
    initVM();
    if (argc == 1)
    {
        repl();
    }
    else if (argc == 2)
    {
        runFile(argv[1]);
    }
    else
    {
        // Too much arguments (>1)
        fprintf(stderr, "Usage: Cellox [path]\n");
        freeVM();
        exit(64);
    }
    freeVM();
    return 0;
}

static char *readFile(const char *path)
{
    // Opens a file of a nonspecified format (b) in read mode (r)
    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    char *buffer = (char *)malloc(fileSize + 1);
    if (buffer == NULL)
    {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize)
    {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    // We add null the end of the sourcecode to mark the end of the file
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void repl()
{
    /// Used to store the next line that read from input
    char line[MAX_LINE_LENGTH];
    for (;;)
    {
        // Prints command prompt
        printf("> ");
        // Reads the next line that was input by the user and stores
        if (!fgets(line, sizeof(line), stdin))
        {
            printf("\n");
            break;
        }
        // We close the command prompt if the last input was empty - \n
        if (strlen(line) == 1)
        {
            exit(0);
        }
        // Interprets the line
        interpret(line);
    }
}

static void runFile(const char *path)
{
    char *source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    // Error during compilation process
    if (result == INTERPRET_COMPILE_ERROR)
        exit(65);
    /// Error during runtime
    if (result == INTERPRET_RUNTIME_ERROR)
        exit(70);
}