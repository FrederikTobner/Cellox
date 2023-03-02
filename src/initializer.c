/****************************************************************************
 * Copyright (C) 2022 by Frederik Tobner                                    *
 *                                                                          *
 * This file is part of Cellox.                                             *
 *                                                                          *
 * Permission to use, copy, modify, and distribute this software and its    *
 * documentation under the terms of the GNU General Public License is       *
 * hereby granted.                                                          *
 * No representations are made about the suitability of this software for   *
 * any purpose.                                                             *
 * It is provided "as is" without express or implied warranty.              *
 * See the <https://www.gnu.org/licenses/gpl-3.0.html/>GNU General Public   *
 * License for more details.                                                *
 ****************************************************************************/

/**
 * @file initializer.c
 * @brief File containing implementation of the behavoir that is used to initialize the interpreter
 * @details The interpreter can be initialized to run from a file or as repl.
 */

#include "initializer.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "backend/virtual_machine.h"
#include "byte-code/chunk.h"
#include "byte-code/chunk_disassembler.h"
#include "byte-code/chunk_file.h"
#include "cellox_config.h"
#include "common.h"
#include "frontend/compiler.h"

/// Maximum length of a line is 1024 characters
#define MAX_LINE_LENGTH (1024u)
/// Lettering that is printed if interpreter is initialized in repl mode
#define PROJECT_INIT_LETTERING \
    ("   _____     _ _           \n\
  / ____|   | | |          \n\
 | |     ___| | | _____  __\n\
 | |    / _ \\ | |/ _ \\ \\/ /\n\
 | |___|  __/ | | (_) >  < \n\
  \\_____\\___|_|_|\\___/_/\\_\\\n")

static void initializer_io_error(char const *, ...);
static char * initializer_read_file(char const *);

void initializer_run_as_repl() {
    virtual_machine_init();
    // Used to store the next line that is read from input
    char line[MAX_LINE_LENGTH];
    printf("%s\t\t Version %i.%i\n", PROJECT_INIT_LETTERING, PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR);
    for (;;) {
        // Prints command prompt
        printf("> ");
        // Reads the next line that was input by the user and stores
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        // We close the command prompt if the last input was empty - \n
        if (strlen(line) == 1) {
            virtual_machine_free();
            exit(EXIT_CODE_OK);
        }
        virtual_machine_interpret(line, false);
    }
}

void initializer_run_from_file(char const * path, bool compile) {
    size_t pathLength = strlen(path);
    interpret_result result;
    // Cellox file -> we need to compile the source code to a chunk in order to execute it
    if (pathLength > 4 && !strcmp(path + pathLength - 4, ".clx")) {
        char * source = initializer_read_file(path);
        if (!source) {
            return;
        }
        virtual_machine_init();
        if (compile) {
            object_function_t * function = compiler_compile(source);
            if (!function) {
                result = INTERPRET_COMPILE_ERROR;
            }
            if (chunk_file_store(function->chunk, path, 0)) {
                result = INTERPRET_COMPILE_ERROR;
            } else {
                result = INTERPRET_OK;
            }
        } else {
            result = virtual_machine_interpret(source, true);
        }
    }
    // Cellox chunk file -> the program has already been compiled
    else if (pathLength > 5 && !strcmp(path + pathLength - 5, ".cxcf")) {
        if (compile) {
            initializer_io_error("Can not compile a chunk file");
        }
        virtual_machine_init();
        result = virtual_machine_run_chunk(*chunk_file_load(path));
    } else {
        initializer_io_error("File type not supported");
        return;
    }

#ifndef CELLOX_TESTS_RUNNING
    if (result != INTERPRET_OK) {
        virtual_machine_free();
    }
    // Error during the compilation process
    if (result == INTERPRET_COMPILE_ERROR) {
        exit(EXIT_CODE_COMPILATION_ERROR);
    }
    // Error at runtime
    if (result == INTERPRET_RUNTIME_ERROR) {
        exit(EXIT_CODE_RUNTIME_ERROR);
    }
#endif
    virtual_machine_free();
}

void initializer_show_help() {
    printf("%s Help\n%s\n\n", PROJECT_NAME, CELLOX_USAGE_MESSAGE);
    printf("Options\n");
    printf("  -c, --compile\t\tConverts the specified file to bytecode and stores the result as a seperate file\n");
    printf("  -h, --help\t\tDisplay this help and exit\n");
    printf("  -v, --version\t\tShows the version of the installed interpreter and exit\n\n");
}

void initializer_show_version() {
    printf("%s Version %i.%i\n", PROJECT_NAME, PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR);
}

/// @brief Prints a error message for io errors and exits if no tests are executed
/// @param format The formater of the error message
/// @param ... The arguments that are formated
static void initializer_io_error(char const * format, ...) {
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
/// @return The contents of the file or NULL if something went wrong an there are tests executed, so we dont want to
/// exit
static char * initializer_read_file(char const * path) {
    // Opens a file of a nonspecified format (b) in read mode (r)
    FILE * file = fopen(path, "rb");
    if (!file) {
        initializer_io_error("Could not open file \"%s\".\n", path);
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
    if (!buffer) {
        initializer_io_error("Not enough memory to read \"%s\".\n", path);
        return NULL;
    }
    // Store amount of read bytes
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        initializer_io_error("Could not read file \"%s\".\n", path);
        return NULL;
    }
    // We add null the end of the source-code to mark the end of the file
    buffer[bytesRead] = '\0';
    fclose(file);
    return buffer;
}
