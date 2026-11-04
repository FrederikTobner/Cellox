#include "disassembler.h"

#include <stdio.h>
#include <stdlib.h>

#include "backend/virtual_machine.h"
#include "base/common.h"
#include "byte-code/chunk_disassembler.h"
#include "byte-code/chunk_file.h"
#include "frontend/compiler.h"

static char * disassembler_read_file(char const * path);

void disassembler_disassemble_file(char const * filepath) {
    size_t pathLength = strlen(filepath);
    if (pathLength > 4 && !strcmp(filepath + pathLength - 4, ".clx")) {
        virtual_machine_init();
        object_function_t * main = compiler_compile(disassembler_read_file(filepath));
        if (!main) {
            printf("Could not compile file \"%s\".\n", filepath);
            exit(EXIT_CODE_COMPILATION_ERROR);
        }
        chunk_disassembler_disassemble_chunk(&main->chunk, "main", main->arity);
        virtual_machine_free();
    } else if (pathLength > 5 && !strcmp(filepath + pathLength - 5, ".cxcf")) {
        virtual_machine_init();
        chunk_t * chunk = chunk_file_load(filepath);
        if (!chunk) {
            printf("Could not read chunk file \"%s\".\n", filepath);
            exit(EXIT_CODE_INPUT_OUTPUT_ERROR);
        }
        chunk_disassembler_disassemble_chunk(chunk, "main", 0);
        virtual_machine_free();
        chunk_free(chunk);
    } else {
        disassembler_show_usage();
    }
}

void disassembler_show_usage(void) {
    printf("Usgae:\nCelloxDisassembler (<celloxfile>|<celloxchunkfile>)");
}

/// @brief Reads a file from disk
/// @param path The path of the file
/// @return The contents of the file or NULL if something went wrong an there are tests executed, so we dont want to
/// exit
static char * disassembler_read_file(char const * path) {
    FILE * file = fopen(path, "rb");
    if (!file) {
        printf("Could not read file");
        exit(EXIT_CODE_INPUT_OUTPUT_ERROR);
    }
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    char * buffer = (char *)malloc(fileSize + 1);
    if (!buffer) {
        printf("Not enough memory to read \"%s\".\n", path);
        exit(EXIT_CODE_INPUT_OUTPUT_ERROR);
    }
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        printf("Could not read file \"%s\".\n", path);
        exit(EXIT_CODE_INPUT_OUTPUT_ERROR);
    }
    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}
