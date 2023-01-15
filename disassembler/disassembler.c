#include "disassembler.h"

#include <stdio.h>
#include <stdlib.h>

#include "../src/chunk_file.h"
#include "../src/common.h"
#include "../src/compiler.h"
#include "../src/debug.h"
#include "../src/virtual_machine.h"

static char * disassembler_read_file(char const * path);

void disassembler_disassemble_file(char const * filepath)
{
    // Determine file type
    size_t pathLength = strlen(filepath);
    if(pathLength > 4 && 
        !strcmp(filepath + pathLength - 4, ".clx"))
    {
        virtual_machine_init();
        object_function_t * main = compiler_compile(disassembler_read_file(filepath));
        debug_disassemble_chunk(&main->chunk, "main", main->arity);
        virtual_machine_free();
    }
    else if(pathLength > 5 &&
            !strcmp(filepath + pathLength - 5, ".cxcf"))
    {
        virtual_machine_init();
        chunk_t * chunk = chunk_file_load(filepath);
        debug_disassemble_chunk(chunk, "main", 0);
        virtual_machine_free();
        chunk_free(chunk);
    }
    else 
    {
        disassembler_show_usage();
    }
}

void disassembler_show_usage()
{
    printf("Usgae:\nCelloxDisassembler (<celloxfile>|<celloxchunkfile>)");
}

/// @brief Reads a file from disk
/// @param path The path of the file
/// @return The contents of the file or NULL if something went wrong an there are tests executed, so we dont want to exit
static char * disassembler_read_file(char const * path)
{
    // Opens a file of a nonspecified format (b) in read mode (r)
    FILE * file = fopen(path, "rb");
    if (!file)
    {
        printf("Could not read file");
        exit(EXIT_CODE_INPUT_OUTPUT_ERROR);
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
        printf("Not enough memory to read \"%s\".\n", path);
        exit(EXIT_CODE_INPUT_OUTPUT_ERROR);
    }
    // Store amount of read bytes
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize)
    {
        printf("Could not read file \"%s\".\n", path);
        exit(EXIT_CODE_INPUT_OUTPUT_ERROR);
    }
    // We add null the end of the source-code to mark the end of the file
    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}
