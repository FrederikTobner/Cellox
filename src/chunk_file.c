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
 * @file chunk_file.c
 * @brief File containing the implementation of functionalitity regarding cellox files.
 */

#include "chunk_file.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "cellox_config.h"

#include "object.h"


/// @brief Chunk segment prefixes
enum chunk_segment_prefix
{
    /// Prefix of a bytecode segment
    CHUNK_SEGMENT_TYPE_BYTECODE = 3,
    /// Prefix of a constant segment
    CHUNK_SEGMENT_TYPE_CONSTANTS = 0,
    /// @brief Prefix of a inner seqment
    /// @details These are functions that are nested inside another function or script
    CHUNK_SEGMENT_TYPE_INNER = 2,
    /// Prefix of a line info segment
    CHUNK_SEGMENT_TYPE_LINE_INFO = 1,
};

/// @brief Constant prefixes for cellox
enum constant_type_prefix
{
    /// Prefix of a numerical constant
    CONSTANT_TYPE_NUMBER,
    /// Prefix of a string constant
    CONSTANT_TYPE_STRING
};

static void chunk_file_append_chunk(chunk_t, chunk_file_compile_flag, FILE *);
static void chunk_file_append_code_segment(uint8_t *, uint32_t, FILE *);
static void chunk_file_append_constant(value_t,  dynamic_value_array_t *, chunk_file_compile_flag, FILE *);
static void chunk_file_append_constant_segment(dynamic_value_array_t, dynamic_value_array_t *, chunk_file_compile_flag, FILE *);
static void chunk_file_append_function_meta_data(object_function_t, FILE *);
static void chunk_file_append_inner_segment(dynamic_value_array_t, chunk_file_compile_flag, FILE *);
static inline void chunk_file_append_line_info(line_info_t, FILE *);
static void chunk_file_append_line_info_segment(line_info_t *, uint32_t, FILE *);
static void chunk_file_append_meta_data(chunk_file_compile_flag, FILE *);
static void chunk_file_append_u32(uint32_t, FILE *);
static void chunk_file_append_u64(uint64_t, FILE *);
static void chunk_file_parse_chunk(char const **, chunk_t *, size_t *, size_t);
static void chunk_file_parse_code(char const **, chunk_t *, size_t *, size_t);
static void chunk_file_parse_constant(char const **, chunk_t *, size_t *, size_t);
static void chunk_file_parse_constants(char const **, chunk_t *, size_t *, size_t);
static void chunk_file_parse_file(char const *, chunk_t *, size_t *, size_t);
static void chunk_file_parse_inner(char const **, chunk_t *, size_t *, size_t);
static void chunk_file_parse_line_info(char const **, chunk_t *, size_t *, size_t);
static void chunk_file_parse_metadata(char const **, chunk_t *, size_t *, size_t);
static uint32_t chunk_file_parse_u32(char const **, chunk_t *, size_t *, size_t);
static uint64_t chunk_file_parse_u64(char const **, chunk_t *, size_t *, size_t);
static char * chunk_file_read_file(char const *, size_t *);
static void chunk_file_error(char const *, ...);

int chunk_file_store(chunk_t chunk, char const * programmPath, chunk_file_compile_flag flag)
{        
    if(flag & COMPILE_FLAG_LINE_INFO_INCLUDED || flag & COMPILE_FLAG_ANONYMIZE_FUNCTIONS || flag & COMPILE_FLAG_OPTIMIZE )
    {
        fprintf(stderr, "Compile options not implemented yet");
        return -1;
    }
    FILE * filePointer;
    size_t fileNameLength = strlen(programmPath) + 2;
    char * filename = malloc(fileNameLength);
    for (size_t i = 0; i < fileNameLength - 2; i++)
        filename[i] = programmPath[i];
    filename[fileNameLength - 4] = 'x';
    filename[fileNameLength - 3] = 'c';
    filename[fileNameLength - 2] = 'f';
    filename[fileNameLength - 1] = '\0';
    filePointer = fopen(filename, "wb");

    if(!filePointer)
    {
        printf("Unable to create file.\n");
        return - 1;
    }
    chunk_file_append_meta_data(flag, filePointer);
    chunk_file_append_chunk(chunk, flag, filePointer);

    fclose(filePointer);
    free(filename);
    return 0;
}

chunk_t * chunk_file_load(char const * filePath)
{
    chunk_t * mainChunk = malloc(sizeof(chunk_t));
    if(!mainChunk)
        return mainChunk;
    size_t fileSize = 0;
    size_t bytesRead = 0;
    char * chunkFileContent = chunk_file_read_file(filePath, &fileSize);
    chunk_file_parse_file(chunkFileContent, mainChunk, &bytesRead, fileSize);
    return mainChunk;
}

/**
 * @brief Writes a single chunk and all its child chunks to a file
 * @details A chunk that is stored in a file has the following structure
 * <ol>
 * <li>----constants-----</li>
 * <li>----lineInfo------</li>
 * <li>----inner chunks--</li>
 * <li>----bytecode------</li>
 * </ol>
 * @param chunk The chunk that is written to a file
 * @param flags Compile flags used to compile the sourcecode to a cellox chunk file
 * @param filePointer Pointer to the file
 */
static void chunk_file_append_chunk(chunk_t chunk,  chunk_file_compile_flag flag, FILE * filePointer)
{
    dynamic_value_array_t functions;
    dynamic_value_array_init(&functions);
    chunk_file_append_constant_segment(chunk.constants, &functions, flag, filePointer);
    chunk_file_append_line_info_segment(chunk.lineInfos, chunk.lineInfoCount, filePointer);
    if(functions.count)
        chunk_file_append_inner_segment(functions, flag, filePointer);        
    chunk_file_append_code_segment(chunk.code, chunk.byteCodeCount, filePointer);
}

/// @brief Appends the bytecode stored in a chunk to the file
/// @param code Pointer to the bytecode of the chunk
/// @param codeSize The amount of bytecode instructions stored in the chunk
/// @param filePointer Pointer to the file
static void chunk_file_append_code_segment(uint8_t * code, uint32_t codeSize, FILE * filePointer)
{
    fputc(CHUNK_SEGMENT_TYPE_BYTECODE, filePointer);
    chunk_file_append_u32(codeSize, filePointer);
    for (uint32_t i = 0; i < codeSize; i++)
        fputc(code[i], filePointer);
}


/// @brief Appends a single constant to a cellox bytecode file
/// @param value The value of the constant that is appended
/// @param functions Pointer to an dynamic array of functions that are used to build the inner segmment of the chunk later
/// @param flags Compile flags used to compile the sourcecode to a cellox chunk file
/// @param filePointer Pointer to the file
static void chunk_file_append_constant(value_t value, dynamic_value_array_t * functions, chunk_file_compile_flag flag, FILE * filePointer)
{
    if(IS_OBJECT(value))
    {
        switch (OBJECT_TYPE(value))
        {
            case OBJECT_FUNCTION:
                dynamic_value_array_write(functions, value);
                break;
            case OBJECT_STRING:
                fputc(CONSTANT_TYPE_STRING, filePointer);
                fputs(AS_CSTRING(value), filePointer);
                fputc(0, filePointer);
                break;
            default:
                fprintf(stderr, "Object type not spupported");
                exit(EXIT_CODE_COMPILATION_ERROR);
        }
    }
    else // Numbers
    {
        fputc(CONSTANT_TYPE_NUMBER, filePointer);
        chunk_file_append_u64(AS_NUMBER(value), filePointer);
    }
}

/// @brief Appends the constants segment of a chunk to the file
/// @param constants The constants that are stored in the chunk
/// @param functions Pointer to an dynamic array of functions that are used to build the inner segmment of the chunk later
/// @param flags Compile flags used to compile the sourcecode to a cellox chunk file
/// @param filePointer Pointer to the file
static void chunk_file_append_constant_segment(dynamic_value_array_t constants, dynamic_value_array_t * functions, chunk_file_compile_flag flag, FILE * filePointer)
{
    if(!constants.count)
        return;
    fputc(CHUNK_SEGMENT_TYPE_CONSTANTS, filePointer);
    uint32_t constantsCounter = constants.count;
    for (size_t i = 0; i < constants.count; i++)
    {
        if(IS_FUNCTION(constants.values[i]))
            constantsCounter--;
    }
    
    chunk_file_append_u32(constantsCounter, filePointer);
    for (size_t i = 0; i < constants.count; i++)   
        chunk_file_append_constant(constants.values[i], functions, flag, filePointer);
}

/// @brief Appends the meta data of a function to the file (arity and upvalue count)
/// @param function The function that has it's metadata appended to the file 
/// @param filePointer Pointer to the file
static void chunk_file_append_function_meta_data(object_function_t function, FILE * filePointer)
{
    fputs(function.name->chars, filePointer);
    fputc(0, filePointer);
    chunk_file_append_u32(function.arity, filePointer);
    chunk_file_append_u32(function.upvalueCount, filePointer);
}

/// @brief Appends the inner segment of the chunk to tthe file
/// @param functions The inner functions of the chunk
/// @param flags Compile flags used to compile the sourcecode to a cellox chunk file
/// @param filePointer Pointer to the file
static void chunk_file_append_inner_segment(dynamic_value_array_t functions, chunk_file_compile_flag flag, FILE * filePointer)
{
    if(!functions.count)
        return;
    fputc(CHUNK_SEGMENT_TYPE_INNER, filePointer);
    chunk_file_append_u32(functions.count, filePointer);
    for (size_t i = 0; i < functions.count; i++)
    {
        chunk_file_append_function_meta_data(*AS_FUNCTION(functions.values[i]), filePointer);
        chunk_file_append_chunk(AS_FUNCTION(functions.values[i])->chunk, flag, filePointer);
    }
}

/// @brief Appends a single line information to a chunk file
/// @param lineInfo The line info that is appended
/// @param filePointer Pointer to the chunk file where the line info is appended
static inline void chunk_file_append_line_info(line_info_t lineInfo, FILE * filePointer)
{
    chunk_file_append_u32(lineInfo.lineNumber, filePointer);
    chunk_file_append_u32(lineInfo.lastOpCodeIndexInLine, filePointer);
}

/// @brief Appends a line information segment to a chunk file
/// @param lineInfos Pointer to the first line information in the array
/// @param lineInfoCount Amount of line info's stored in the line information array
/// @param filePointer Pointer to the chunk file where the line info is appended
static void chunk_file_append_line_info_segment(line_info_t * lineInfos, uint32_t lineInfoCount, FILE * filePointer)
{
    if(!lineInfoCount)
        return;
    fputc(CHUNK_SEGMENT_TYPE_LINE_INFO, filePointer);
    chunk_file_append_u32(lineInfoCount, filePointer);
    for (uint32_t i = 0; i < lineInfoCount; i++)
        chunk_file_append_line_info(lineInfos[i], filePointer);
}

/// @brief Appends the meta data of a cellox bytecode file (chunk file writer options and the cellox version)
/// @param flags Compile flags used to compile the sourcecode to a cellox chunk file
/// @param filePointer Pointer to the file
static void chunk_file_append_meta_data(chunk_file_compile_flag flag, FILE * filePointer)
{
    fputc(flag, filePointer);
    fputc(PROJECT_VERSION_MAJOR, filePointer);    
    fputc(PROJECT_VERSION_MINOR, filePointer);
}

/// @brief Appends a 32bit unsigned integer value to a chunk file
/// @param count The value that is appended
/// @param filePointer Pointer to the chunk file where the value is appended
static void chunk_file_append_u32(uint32_t number, FILE * filePointer)
{
    uint8_t numberShifted = (number & 0xff000000u) >> 24;
    fputc(numberShifted, filePointer);
    numberShifted = (number & 0x00ff0000u) >> 16;
    fputc(numberShifted, filePointer);
    numberShifted = (number & 0x0000ff00u) >> 8;
    fputc(numberShifted, filePointer);
    numberShifted = number  & 0x000000ffu;
    fputc(number & 0x000000ffu, filePointer);
}

/// @brief Appends a 64bit unsigned integer value to a chunk file
/// @param count The value that is appended
/// @param filePointer Pointer to the chunk file where the value is appended
static void chunk_file_append_u64(uint64_t number, FILE * filePointer)
{
    fputc((number & 0xff00000000000000) >> 56, filePointer);
    fputc((number & 0x00ff000000000000) >> 48, filePointer);
    fputc((number & 0x0000ff0000000000) >> 40, filePointer);
    fputc((number & 0x000000ff00000000) >> 32, filePointer);
    fputc((number & 0x00000000ff000000) >> 24, filePointer);
    fputc((number & 0x0000000000ff0000) >> 16, filePointer);
    fputc((number & 0x000000000000ff00) >> 8, filePointer);
    fputc(number  & 0x00000000000000ff, filePointer);
}

/// @brief Parses a chunk in a chunk file
/// @param fileContent  Pointer to the pointer where the contents of the file are stored
/// @param result The resulting chunk of the parsing process
/// @param bytesReadPointer Pointer to the bytes read counter
/// @param fileSize The size of the file in bytes
static void chunk_file_parse_chunk(char const ** fileContent, chunk_t * result, size_t * bytesReadPointer, size_t fileSize)
{
    if(*bytesReadPointer != fileSize && (**fileContent) == CHUNK_SEGMENT_TYPE_CONSTANTS)
    {
        (*bytesReadPointer)++;
        (*fileContent)++;
        chunk_file_parse_constants(fileContent, result, bytesReadPointer, fileSize);
    }
    if(*bytesReadPointer != fileSize && (**fileContent) == CHUNK_SEGMENT_TYPE_LINE_INFO)
    {
        (*bytesReadPointer)++;
        (*fileContent)++;
        chunk_file_parse_line_info(fileContent, result, bytesReadPointer, fileSize);
    }
    if(*bytesReadPointer != fileSize && (**fileContent) == CHUNK_SEGMENT_TYPE_INNER)
    {
        (*bytesReadPointer)++;
        (*fileContent)++;
        chunk_file_parse_inner(fileContent, result, bytesReadPointer, fileSize);
    }
    if(*bytesReadPointer != fileSize && (**fileContent) == CHUNK_SEGMENT_TYPE_BYTECODE)
    {
        (*bytesReadPointer)++;
        (*fileContent)++;
        chunk_file_parse_code(fileContent, result, bytesReadPointer, fileSize);
    }
    else {
        result->byteCodeCapacity = 0;
        result->byteCodeCount = 0;
    }
}

/// @brief Parses the bytecode segment of a chunk in a chunk file
/// @param fileContent Pointer to the pointer where the contents of the file are stored
/// @param result The resulting chunk of the parsing process
/// @param bytesReadPointer Pointer to the bytes read counter
/// @param fileSize The size of the file in bytes
static void chunk_file_parse_code(char const ** fileContent, chunk_t * result, size_t * bytesReadPointer, size_t fileSize)
{
    uint32_t codeCount = chunk_file_parse_u32(fileContent, result, bytesReadPointer, fileSize);
    if (!codeCount)
        return;
    uint32_t codeAbsoluteSize = fileSize - *bytesReadPointer;
    result->code = malloc(sizeof(uint8_t) * codeAbsoluteSize);
    if (!result->code)
        chunk_file_error("Could not create line info");
    result->byteCodeCapacity = codeCount;
    result->byteCodeCount = codeCount;
    for (uint32_t i = 0; i < codeAbsoluteSize; i++, (*bytesReadPointer)++)
        result->code[i] = *(*fileContent)++;
}

/// @brief Parses a single constant in a constant segment
/// @param fileContent Pointer to the pointer where the contents of the file are stored
/// @param result The resulting chunk of the parsing process
/// @param bytesReadPointer Pointer to the bytes read counter
/// @param fileSize The size of the file in bytes
static void chunk_file_parse_constant(char const ** fileContent, chunk_t * result, size_t * bytesReadPointer, size_t fileSize)
{
    if (*bytesReadPointer > fileSize)
        chunk_file_error("Unexpected file ending");
    switch (*(*fileContent)++)
    {
    case CONSTANT_TYPE_NUMBER:
        dynamic_value_array_write(&result->constants, NUMBER_VAL(chunk_file_parse_u64(fileContent, result, bytesReadPointer, fileSize)));
        break;
    case CONSTANT_TYPE_STRING:
    {
        size_t stringLength = strlen(*fileContent);
        if (*bytesReadPointer > fileSize - stringLength)
            chunk_file_error("Unexpected file ending");
        char * stringLiteral = malloc(stringLength + 1);
        strcpy(stringLiteral, *fileContent);
        stringLiteral[stringLength] = '\0';
        dynamic_value_array_write(&result->constants, OBJECT_VAL(object_take_string(stringLiteral, stringLength)));
        *fileContent += stringLength + 1;
        *bytesReadPointer += stringLength + 2;
        break;
    }
    
    default:
        chunk_file_error("Unknown constant type");
    }   
}

/// @brief Parses the constants segment of a chunk in a chunk file
/// @param fileContent Pointer to the pointer where the contents of the file are stored
/// @param result The resulting chunk of the parsing process
/// @param bytesReadPointer Pointer to the bytes read counter
/// @param fileSize The size of the file in bytes
static void chunk_file_parse_constants(char const ** fileContent, chunk_t * result, size_t * bytesReadPointer, size_t fileSize)
{
    uint32_t constantCounter = chunk_file_parse_u32(fileContent, result, bytesReadPointer, fileSize);
    dynamic_value_array_init(&result->constants);
    for (uint32_t i = 0; i < constantCounter; i++)
        chunk_file_parse_constant(fileContent, result, bytesReadPointer, fileSize);    
}

/// @brief Parses a cellox chunk file
/// @param fileContent Pointer to the contents of the file
/// @param result The resulting chunk of the parsing process
/// @param bytesReadPointer Pointer to the bytes read counter
/// @param fileSize The size of the file in bytes
static void chunk_file_parse_file(char const * fileContent, chunk_t * result, size_t * bytesReadPointer, size_t fileSize)
{
    chunk_file_parse_metadata(&fileContent, result, bytesReadPointer, fileSize);
    chunk_file_parse_chunk(&fileContent, result, bytesReadPointer, fileSize);
    // We miss reading 2 bytes somewhere for counter.clx :(
    /*if(*bytesReadPointer != fileSize)
        chunk_file_error("Could not parse the whole file");*/
}

/// @brief Parses the inner segment of a chunk in a chunk file
/// @param fileContent Pointer to the pointer where the contents of the file are stored
/// @param result The resulting chunk of the parsing process
/// @param bytesReadPointer Pointer to the bytes read counter
/// @param fileSize The size of the file in bytes
static void chunk_file_parse_inner(char const ** fileContent, chunk_t * result, size_t * bytesReadPointer, size_t fileSize)
{
    uint32_t innerCounter = chunk_file_parse_u32(fileContent, result, bytesReadPointer, fileSize);
    for (size_t i = 0; i < innerCounter; i++)
    {
        size_t functionNameLength = strlen(*fileContent);
        if (*bytesReadPointer > fileSize - functionNameLength)
            chunk_file_error("Unexpected file ending");
        object_function_t * function = object_new_function();
        char * functionName = malloc(functionNameLength + 1);
        memcpy(functionName, (*fileContent), functionNameLength);
        functionName[functionNameLength] = '\0';
        function->name = object_take_string(functionName, functionNameLength);
        *fileContent += functionNameLength + 1;
        *bytesReadPointer += functionNameLength + 2;
        function->arity = chunk_file_parse_u32(fileContent, result, bytesReadPointer, fileSize);
        function->upvalueCount = chunk_file_parse_u32(fileContent, result, bytesReadPointer, fileSize);
        chunk_file_parse_chunk(fileContent, &function->chunk, bytesReadPointer, fileSize);
        dynamic_value_array_write(&result->constants, OBJECT_VAL(function));
    }    
}

/// @brief Parses the line info of a chunk
/// @param fileContent Pointer to the pointer where the contents of the file are stored
/// @param result The resulting chunk of the parsing process
/// @param bytesReadPointer Pointer to the bytes read counter
/// @param fileSize The size of the file in bytes
static void chunk_file_parse_line_info(char const ** fileContent, chunk_t * result, size_t * bytesReadPointer, size_t fileSize)
{
    uint32_t lineInfoCount = chunk_file_parse_u32(fileContent, result, bytesReadPointer, fileSize);
    if(!lineInfoCount)
        return;
    result->lineInfos = malloc(sizeof(line_info_t) * lineInfoCount);
    if (!result->lineInfos)
        chunk_file_error("Could not create line info");
    result->lineInfoCount = lineInfoCount;
    result->lineInfoCapacity = lineInfoCount;
    for (uint32_t i = 0; i < lineInfoCount; i++)
    {
        result->lineInfos[i].lineNumber = chunk_file_parse_u32(fileContent, result, bytesReadPointer, fileSize);
        result->lineInfos[i].lastOpCodeIndexInLine = chunk_file_parse_u32(fileContent, result, bytesReadPointer, fileSize);
    }
}

/// @brief Parses the metadata of a chunk file
/// @param fileContent Pointer to the pointer where the contents of the file are stored
/// @param result The resulting chunk of the parsing process
/// @param bytesReadPointer Pointer to the bytes read counter
/// @param fileSize The size of the file in bytes
static void chunk_file_parse_metadata(char const ** fileContent, chunk_t * result, size_t * bytesReadPointer, size_t fileSize)
{
    for (; (*bytesReadPointer) < 3; (*bytesReadPointer)++, (*fileContent)++)
    {
        if ((*bytesReadPointer) >= fileSize)
            chunk_file_error("Chunk file is incomplete");
        // We ignore the metadata right now
    }    
}

/// @brief Parses a single unsigned 32-bit integer value
/// @param fileContent Pointer to the pointer where the contents of the file are stored
/// @param result The resulting chunk of the parsing process
/// @param bytesReadPointer Pointer to the bytes read counter
/// @param fileSize The size of the file in bytes
/// @return The number that was parsed
static uint32_t chunk_file_parse_u32(char const ** fileContent, chunk_t * result, size_t * bytesReadPointer, size_t fileSize)
{
    if ((*bytesReadPointer) > (fileSize - 4))
        chunk_file_error("Chunk file is incomplete");
    uint32_t number = 0;
    number += (*(*fileContent)++) << 24;
    number += (*(*fileContent)++) << 16;
    number += (*(*fileContent)++) << 8;
    number += (*(*fileContent)++);
    (*bytesReadPointer) += 4;
    return number;
}

/// @brief Parses a single unsigned 64-bit integer value
/// @param fileContent Pointer to the pointer where the contents of the file are stored
/// @param result The resulting chunk of the parsing process
/// @param bytesReadPointer Pointer to the bytes read counter
/// @param fileSize The size of the file in bytes
/// @return The number that was parsed
static uint64_t chunk_file_parse_u64(char const ** fileContent, chunk_t * result, size_t * bytesReadPointer, size_t fileSize)
{
    if((*bytesReadPointer) > (fileSize - 8))
        chunk_file_error("Chunk file is incomplete");
    uint64_t number = 0;
    number += ((uint64_t) *(*fileContent)++) << 56;
    number += ((uint64_t) *(*fileContent)++) << 48;
    number += ((uint64_t) *(*fileContent)++) << 40;
    number += ((uint64_t) *(*fileContent)++) << 32;
    number += ((uint64_t) *(*fileContent)++) << 24;
    number += ((uint64_t) *(*fileContent)++) << 16;
    number += ((uint64_t) *(*fileContent)++) << 8;
    number += ((uint64_t) *(*fileContent)++);
    (*bytesReadPointer) += 8;
    return number;
}

/// @brief Reads a file from disk
/// @param path The path of the file
/// @return The contents of the file
static char * chunk_file_read_file(char const * path, size_t * fileSizePointer)
{
    // Opens a file of a nonspecified format (b) in read mode (r)
    FILE * file = fopen(path, "rb");
    if (!file)
        chunk_file_error("Could not open chunk file \"%s\".\n", path);
    // Seek end of the file
    fseek(file, 0L, SEEK_END);
    // Store filesize
    size_t fileSize = ftell(file);
    // Rewind filepointer to the beginning of the file
    rewind(file);
    // Allocate memory apropriate to store the file
    char * buffer = (char *)malloc(fileSize + 1);
    if (!buffer)
        chunk_file_error("Not enough memory to read \"%s\".\n", path);
    // Store amount of read bytes
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize)
        chunk_file_error("Could not read chunk file \"%s\".\n", path);
    fclose(file);
    *fileSizePointer = bytesRead;
    return buffer;
}

/// @brief Prints a error message for io errors and exits
/// @param format The formater of the error message
/// @param ... The arguments that are formated
static void chunk_file_error(char const * format, ...)
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
