#include "init.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "cellox_config.h"
#include "chunk_file.h"
#include "common.h"
#include "compiler.h"
#include "chunk.h"
#include "debug.h"
#include "virtual_machine.h"

/// Maximum length of a line is 1024 characters
#define MAX_LINE_LENGTH (1024u)

static void init_io_error(char const *, ...);
static char * init_read_file(char const *);

void init_repl()
{
    virtual_machine_init();
    // Used to store the next line that is read from input
    char line[MAX_LINE_LENGTH];
    printf("   _____     _ _           \n\
  / ____|   | | |          \n\
 | |     ___| | | _____  __\n\
 | |    / _ \\ | |/ _ \\ \\/ /\n\
 | |___|  __/ | | (_) >  < \n\
  \\_____\\___|_|_|\\___/_/\\_\\\n");
  
// The cellox_config.h.in file is not configured by cmake for the benchmarks and tests -> so we just ignore it
    printf("\t\t Version %i.%i\n", PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR);
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
            virtual_machine_free();
            exit(EXIT_CODE_OK);            
        }
        virtual_machine_interpret(line, false);
    }
}

void init_run_from_file(char const * path, bool compile)
{
    virtual_machine_init();
    char * source = init_read_file(path);
    #ifdef CELLOX_TESTS_RUNNING
    if(!source)
        return;
    #endif
    interpret_result_t result;
    if(compile)
    {
        object_function_t * function = compiler_compile(source);        
        if (!function)
            result = INTERPRET_COMPILE_ERROR;
        if(chunk_file_store(function->chunk, path, 0))
            result = INTERPRET_COMPILE_ERROR;
        else
            result = INTERPRET_OK;
    }
    else
        result = virtual_machine_interpret(source, true);
    #ifndef CELLOX_TESTS_RUNNING
    if(result != INTERPRET_OK)
        virtual_machine_free();
    // Error during compilation process
    if (result == INTERPRET_COMPILE_ERROR)
        exit(EXIT_CODE_COMPILATION_ERROR);
    // Error during runtime
    if (result == INTERPRET_RUNTIME_ERROR)
        exit(EXIT_CODE_RUNTIME_ERROR);
    #endif
    virtual_machine_free();
}

void init_show_help()
{
    #ifndef BENCHMARKS_RUNNING
    #ifndef CELLOX_TESTS_RUNNING
    printf("%s Help\n%s\n", PROJECT_NAME, CELLOX_USAGE_MESSAGE);
    #endif
    #endif
}

void init_show_version()
{
    printf("%s Version %i.%i\n", PROJECT_NAME, PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR);
}

/// @brief Prints a error message for io errors and exits if no tests are executed
/// @param format The formater of the error message
/// @param ... The arguments that are formated
static void init_io_error(char const * format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
    #ifndef CELLOX_TESTS_RUNNING
        exit(EXIT_CODE_INPUT_OUTPUT_ERROR);
    #endif
}

/// @brief Reads a file from disk
/// @param path The path of the file
/// @return The contents of the file or NULL if something went wrong 😕
static char * init_read_file(char const * path)
{
    // Opens a file of a nonspecified format (b) in read mode (r)
    FILE * file = fopen(path, "rb");
    if (!file)
    {
        init_io_error("Could not open file \"%s\".\n", path);
        return NULL;
    }
    // Seek end of the file
    fseek(file, 0L, SEEK_END);
    // Store filesize
    size_t fileSize = ftell(file);
    // Rewind filepointer to the beginning of the file
    rewind(file);
    // Allocate memory apropriate to store the file
    char * buffer = (char *)malloc(fileSize + 1);
    if (!buffer)
    {
        init_io_error("Not enough memory to read \"%s\".\n", path);
        return NULL;
    }
    // Store amount of read bytes
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize)
    {
        init_io_error("Could not read file \"%s\".\n", path);
        return NULL;
    }
    // We add null the end of the source-code to mark the end of the file
    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}
